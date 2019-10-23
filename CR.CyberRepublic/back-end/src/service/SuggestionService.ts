import Base from './Base'
import * as _ from 'lodash'
import { constant } from '../constant'
import { validate, mail, user as userUtil, permissions, logger } from '../utility'

const BASE_FIELDS = ['title', 'type', 'abstract', 'goal', 'motivation', 'relevance', 'budget', 'plan']

export default class extends Base {
  private model: any
  protected init() {
    this.model = this.getDBModel('Suggestion')
  }

  public async create(param: any): Promise<Document> {
    const doc = {
      ...param,
      createdBy: _.get(this.currentUser, '_id'),
      contentType: constant.CONTENT_TYPE.MARKDOWN,
      // this is a hack for now, we should really be using aggregate pipeline + projection
      // in the sort query
      descUpdatedAt: new Date()
    }
    // save the document
    const result = await this.model.save(doc)
    await this.getDBModel('Suggestion_Edit_History').save({ ...param, suggestion: result._id })

    return result
  }

  // obsolete method
  public async sendMentionEmails(suggestion, mentions) {
    const db_user = this.getDBModel('User')
    const query = { role: constant.USER_ROLE.COUNCIL }
    const councilMembers = await db_user.getDBInstance().find(query)
        .select(constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)

    const subject = 'You were mentioned in a suggestion'
    const body = `
      <p>You were mentioned in a suggestion, click this link to view more details:</p>
      <br />
      <p><a href="${process.env.SERVER_URL}/suggestion/${suggestion._id}">${process.env.SERVER_URL}/suggestion/${suggestion._id}</a></p>
      <br /> <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
    `

    if (_.includes(mentions, '@</span>ALL')) {
      _.map(councilMembers, user => {
        mail.send({
          to: user.email,
          toName: userUtil.formatUsername(user),
          subject,
          body
        })
      })
      return
    }

    // hack for now, don't send more than 1 email to an individual subscriber
    const seenEmails = {}

    for (let mention of mentions) {
      const username = mention.replace('@</span>', '')
      const user = await db_user.findOne({ username })

      const to = user.email
      const toName = userUtil.formatUsername(user)

      if (seenEmails[to]) {
        continue
      }

      await mail.send({
        to,
        toName,
        subject,
        body
      })

      seenEmails[to] = true
    }
  }

  public async update(param: any): Promise<Document> {
    const { id, update } = param
    const userId = _.get(this.currentUser, '_id')
    const currDoc = await this.model.getDBInstance().findById(id)

    if (!currDoc) {
      throw 'Current document does not exist'
    }

    if (!userId.equals(_.get(currDoc, 'createdBy')) && !permissions.isAdmin(_.get(this.currentUser, 'role'))) {
      throw 'Only owner can edit suggestion'
    }

    const doc = _.pick(param, BASE_FIELDS);
    doc.descUpdatedAt = new Date()
    if (update) {
      await Promise.all([
        this.model.update({ _id: id }, { $set: doc }),
        this.getDBModel('Suggestion_Edit_History').save({ ...doc, suggestion: id })
      ])
    } else {
      await this.model.update({ _id: id }, { $set: doc })
    }
    return this.show({ id })
  }

  public async list(param: any): Promise<Object> {
    const query = _.omit(param, ['results', 'page', 'sortBy', 'sortOrder', 'filter', 'profileListFor', 'search', 'tagsIncluded', 'referenceStatus'])
    const { sortBy, sortOrder, tagsIncluded, referenceStatus } = param
    let qryTagsType: any

    query.$or = []
    if (!_.isEmpty(tagsIncluded)) {
      qryTagsType = { $in: tagsIncluded.split(',') }
      query.$or.push({'tags.type': qryTagsType})
    }
    if (referenceStatus === 'true') {
      // if we have another tag selected we only want that tag and referenced suggestions
      query.$or.push({reference: {$exists: true, $ne: []}})
    }

    if (_.isEmpty(query.$or)) delete query.$or
    delete query['tags.type']

    const excludedFields = [
      '-comments', '-goal', '-motivation',
      '-relevance', '-budget', '-plan',
      '-subscribers', '-likes', '-dislikes', '-updatedAt'
    ]

    const sortObject = {}
    let cursor: any
    if (sortBy) {
      // hack to prioritize descUpdatedAt if it's createdAt
      if (sortBy === 'createdAt') {
        sortObject['descUpdatedAt'] = _.get(constant.SORT_ORDER, sortOrder, constant.SORT_ORDER.DESC)
      }
      sortObject[sortBy] = _.get(constant.SORT_ORDER, sortOrder, constant.SORT_ORDER.DESC)

      cursor = this.model.getDBInstance()
        .find(query, excludedFields.join(' '))
        .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)
        .populate('reference', constant.DB_SELECTED_FIELDS.CVOTE.ID_STATUS)
        .sort(sortObject)
    } else {
      cursor = this.model.getDBInstance()
        .find(query)
        .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)
        .populate('reference', constant.DB_SELECTED_FIELDS.CVOTE.ID_STATUS)
    }

    if (param.results) {
      const results = parseInt(param.results, 10)
      const page = parseInt(param.page, 10)
      cursor.skip(results * (page - 1)).limit(results)
    }

    const rs = await Promise.all([
      cursor,
      this.model.getDBInstance().find(query).count()
    ])

    return {
      list: rs[0],
      total: rs[1]
    }
  }

  public async show(param: any): Promise<Document> {
    const { id: _id, incViewsNum } = param
    if (incViewsNum === 'true') {
      await this.model.findOneAndUpdate({ _id }, {
        $inc: { viewsNum: 1, activeness: 1 }
      })
    }
    const doc = await this.model.getDBInstance()
      .findById(_id)
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)
      .populate('reference', constant.DB_SELECTED_FIELDS.CVOTE.ID_STATUS)

    if (_.isEmpty(doc.comments)) return doc

    for (const comment of doc.comments) {
      for (const thread of comment) {
        await this.model.getDBInstance().populate(thread, {
          path: 'createdBy',
          select: `${constant.DB_SELECTED_FIELDS.USER.NAME} profile.avatar`
        })
      }
    }

    return doc
  }

  public async editHistories(param: any): Promise<Document[]> {
    return await this.getDBModel('Suggestion_Edit_History').find({suggestion: param.id})
  }

  // like or unlike
  public async like(param: any): Promise<Document> {
    const { id: _id } = param
    const userId = _.get(this.currentUser, '_id')
    const doc = await this.model.findById(_id)
    const { likes, dislikes } = doc

    // can not both like and dislike, use ObjectId.equals to compare
    if (_.findIndex(dislikes, oid => userId.equals(oid)) !== -1) return doc

    // already liked, will unlike, use ObjectId.equals to compare
    if (_.findIndex(likes, oid => userId.equals(oid)) !== -1) {
      await this.model.findOneAndUpdate({ _id }, {
        $pull: { likes: userId },
        $inc: { likesNum: -1, activeness: -1 }
      })
    } else {
      // not like yet, will like it
      await this.model.findOneAndUpdate({ _id }, {
        $push: { likes: userId },
        $inc: { likesNum: 1, activeness: 1 }
      })
    }

    return this.model.findById(_id)
  }
  // dislike <=> undislike
  // can not both like and dislike
  public async dislike(param: any): Promise<Document> {
    const { id: _id } = param
    const userId = _.get(this.currentUser, '_id')
    const doc = await this.model.findById(_id)
    const { likes, dislikes } = doc

    // can not both like and dislike, use ObjectId.equals to compare
    if (_.findIndex(likes, oid => userId.equals(oid)) !== -1) return doc

    // already liked, will unlike, use ObjectId.equals to compare
    if (_.findIndex(dislikes, oid => userId.equals(oid)) !== -1) {
      await this.model.findOneAndUpdate({ _id }, {
        $pull: { dislikes: userId },
        $inc: { dislikesNum: -1, activeness: -1 }
      })
    } else {
      // not like yet, will like it
      await this.model.findOneAndUpdate({ _id }, {
        $push: { dislikes: userId },
        $inc: { dislikesNum: 1, activeness: 1 }
      })
    }

    return this.model.findById(_id)
  }

  public async reportabuse(param: any): Promise<Document> {
    const { id: _id } = param
    const updateObject = {
      abusedStatus: constant.SUGGESTION_ABUSED_STATUS.REPORTED
    }
    await this.model.findOneAndUpdate({ _id }, updateObject)
    return this.model.findById(_id)
  }

  /**
   * Council only
   */
  private async notifySubscribers(suggestionId: String) {
    try {
      const db_user = this.getDBModel('User');
      const currentUserId = _.get(this.currentUser, '_id')
      const councilMember = await db_user.findById(currentUserId)
      const suggestion = await this.model.getDBInstance().findById(suggestionId)
        .populate('subscribers.user', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)
        .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)

      // get users: creator and subscribers
      const toUsers = _.map(suggestion.subscribers, 'user') || []
      toUsers.push(suggestion.createdBy)
      const toMails = _.map(toUsers, 'email')

      const subject = 'The suggestion is under consideration of Council.'
      const body = `
      <p>Council member ${userUtil.formatUsername(councilMember)} has marked this suggestion ${suggestion.title} as "Under Consideration"</p>
      <br />
      <p>Click this link to view more details: <a href="${process.env.SERVER_URL}/suggestion/${suggestion._id}">${process.env.SERVER_URL}/suggestion/${suggestion._id}</a></p>
      <br /> <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
    `

      const recVariables = _.zipObject(toMails, _.map(toUsers, (user) => {
        return {
          _id: user._id,
          username: userUtil.formatUsername(user)
        }
      }))

      const mailObj = {
        to: toMails,
        // toName: ownerToName,
        subject,
        body,
        recVariables,
      }

      mail.send(mailObj)
    } catch(error) {
      logger.error(error)
    }
  }

  private async notifyOwner(suggestionId: String, desc: String) {
    try {
      const db_user = this.getDBModel('User');
      const currentUserId = _.get(this.currentUser, '_id')
      const councilMember = await db_user.findById(currentUserId)
      const suggestion = await this.model.getDBInstance().findById(suggestionId)
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)

      // get users: creator and subscribers
      const toUsers = [suggestion.createdBy]
      const toMails = _.map(toUsers, 'email')

      const subject = 'Your suggestion needs more info for Council.'
      const body = `
        <p>Council member ${userUtil.formatUsername(councilMember)} has marked your suggestion ${suggestion.title} as "Need more information".</p>
        <br />
        <p>"${desc}"</p>
        <br />
        <p>Click this link to view more details: <a href="${process.env.SERVER_URL}/suggestion/${suggestion._id}">${process.env.SERVER_URL}/suggestion/${suggestion._id}</a></p>
        <br /> <br />
        <p>Thanks</p>
        <p>Cyber Republic</p>
      `

      const recVariables = _.zipObject(toMails, _.map(toUsers, (user) => {
        return {
          _id: user._id,
          username: userUtil.formatUsername(user)
        }
      }))

      const mailObj = {
        to: toMails,
        // toName: ownerToName,
        subject,
        body,
        recVariables,
      }

      mail.send(mailObj)
    } catch (error) {
      logger.error(error)
    }
  }

  public async addTag(param: any): Promise<Document> {
    try {
      const { id: _id, type, desc } = param
      const currDoc = await this.model.getDBInstance().findById(_id)

      if (!currDoc) {
        throw 'Current document does not exist'
      }

      if (_.findIndex(currDoc.tags, (tagObj: any) => tagObj.type === type) !== -1) return currDoc

      const tag: any = {
        type,
        createdBy: _.get(this.currentUser, '_id'),
      }
      if (desc) tag.desc = desc
      const updateObject = {
        $addToSet: { tags: tag }
      }

      await this.model.findOneAndUpdate({ _id }, updateObject)
      if (type === constant.SUGGESTION_TAG_TYPE.UNDER_CONSIDERATION) {
        this.notifySubscribers(_id)
      } else if (type === constant.SUGGESTION_TAG_TYPE.INFO_NEEDED) {
        this.notifyOwner(_id, desc)
      }
      return this.model.findById(_id)
    } catch(error) {
      logger.error(error)
    }
  }

  public async abuse(param: any): Promise<Document> {
    const { id: _id } = param
    const updateObject = {
      status: constant.SUGGESTION_STATUS.ABUSED,
      abusedStatus: constant.SUGGESTION_ABUSED_STATUS.HANDLED
    }
    await this.model.findOneAndUpdate({ _id }, updateObject)
    return this.model.findById(_id)
  }

  public async investigation(param: any): Promise<object> {
    const { id } = param
    const sugg = await this.model.getDBInstance().findById(id)
    if (!sugg) {
      return { success: false }
    }
    const council = userUtil.formatUsername(this.currentUser)
    const subject = `Need background investigation on suggestion #${sugg.displayId}`
    const body = `
      <p>Council member ${council} requested secretary to do background investigation on suggestion #${sugg.displayId}</p>
      <br />
      <p>Click the link to view the suggestion detail: <a href="${
      process.env.SERVER_URL
      }/suggestion/${sugg._id}">${process.env.SERVER_URL}/suggestion/${sugg._id}</a></p>
      <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `

    await this.notifySecretaries(subject, body)
    return { success: true, message: 'Ok' }
  }

  public async advisory(param: any): Promise<object> {
    const { id } = param
    const sugg = await this.model.getDBInstance().findById(id)
    if (!sugg) {
      return { success: false }
    }
    const council = userUtil.formatUsername(this.currentUser)
    const subject = `Need technical advisory on suggestion #${sugg.displayId}`
    const body = `
      <p>Council member ${council} requested secretary to provide advisory on suggestion #${sugg.displayId}</p>
      <br />
      <p>Click the link to view the suggestion detail: <a href="${
      process.env.SERVER_URL
      }/suggestion/${sugg._id}">${process.env.SERVER_URL}/suggestion/${sugg._id}</a></p>
      <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `

    await this.notifySecretaries(subject, body)
    return { success: true, message: 'Ok' }
  }

  private async notifySecretaries(subject: string, body: string): Promise<any> {
    const db_user = this.getDBModel('User')
    const currentUserId = _.get(this.currentUser, '_id')
    const secretaries = await db_user.find({
      role: constant.USER_ROLE.SECRETARY
    })
    const toUsers = _.filter(
      secretaries,
      user => !user._id.equals(currentUserId)
    )
    const toMails = _.map(toUsers, 'email')
    const recVariables = _.zipObject(
      toMails,
      _.map(toUsers, user => {
        return {
          _id: user._id,
          username: userUtil.formatUsername(user)
        }
      })
    )
    const mailObj = {
      to: toMails,
      subject,
      body,
      recVariables
    }
 
    return mail.send(mailObj)
  }

  /**
   * Admin and Author
   */
  public async archive(param: any): Promise<object> {
    const { id: _id } = param
    const suggestion = await this.model.getDBInstance().findById(_id).populate('createdBy')
    if (!suggestion) {
      return
    }
    const isAdmin = this.currentUser.role === constant.USER_ROLE.ADMIN
    const isAuthor = suggestion.createdBy._id.equals(this.currentUser._id)
    if (!(isAdmin || isAuthor)) {
      return
    }
    const updateObject = {
      status: constant.SUGGESTION_STATUS.ARCHIVED,
    }
    await this.model.update({ _id }, updateObject)
    return { success: true, message: 'Ok'}
  }

  public async delete(param: any): Promise<Document> {
    const { id: _id } = param
    return this.model.findByIdAndDelete(_id)
  }

  /**
   * Utils
   */
  public validateTitle(title: String) {
    if (!validate.valid_string(title, 4)) {
      throw 'invalid title'
    }
  }

  public validateDesc(desc: String) {
    if (!validate.valid_string(desc, 1)) {
      throw 'invalid description'
    }
  }
}
