import Base from './Base'
import * as _ from 'lodash'
import { constant } from '../constant'
import { validate, mail, user as userUtil, permissions } from '../utility'

const emptyDoc = {
  title: '',
  shortDesc: '',
  desc: '',
  benefits: '',
  funding: '',
  timeline: undefined,
  link: [],
}

const listExlucdedFields = [
  constant.SUGGESTION_TAG_TYPE.UNDER_CONSIDERATION,
  constant.SUGGESTION_TAG_TYPE.INFO_NEEDED
]

export default class extends Base {
  private model: any
  protected init() {
    this.model = this.getDBModel('Suggestion')
  }

  public async create(param: any): Promise<Document> {
    // get param
    const { title, shortDesc, desc, benefits, funding, timeline, link, } = param
    // validation
    this.validateTitle(title)
    this.validateDesc(desc)

    const docCore: any = {
      title,
      shortDesc,
      desc,
      benefits,
      // funding,
      // timeline,
      // link,
    }
    if (!_.isEmpty(funding)) {
      docCore.funding = funding
    }
    if (!_.isEmpty(timeline)) {
      docCore.timeline = timeline
    }
    if (!_.isEmpty(link)) {
      docCore.link = link
    }

    // console.log('docCore is: ', docCore)

    // build document object
    const doc = {
      ...docCore,
      createdBy: _.get(this.currentUser, '_id'),
      editHistory: [emptyDoc, docCore],
    }
    // save the document
    return await this.model.save(doc)
  }

  public async update(param: any): Promise<Document> {
    // get param
    const { id, title, shortDesc, desc, benefits, funding, timeline, link } = param
    const userId = _.get(this.currentUser, '_id')
    const currDoc = await this.model.getDBInstance().findById(id)

    if (!currDoc) {
      throw 'Current document does not exist'
    }

    if (!userId.equals(_.get(currDoc, 'createdBy')) && !permissions.isAdmin(_.get(this.currentUser, 'role'))) {
      throw 'Only owner can edit suggestion'
    }

    // validation
    this.validateTitle(title)
    this.validateDesc(desc)

    // build document object
    const doc: any = {
      title,
      shortDesc,
      desc,
      benefits,
      funding,
      timeline,
      link,
    }
    if (_.isEmpty(link)) {
      doc.link = []
    }

    // update the document
    if (!_.isEmpty(currDoc.editHistory)) {
      await this.model.update({_id: id}, {$set: doc, $push: { editHistory: doc }})
    } else {
      const firstHistoryItem = {
        title: currDoc.title,
        shortDesc: currDoc.shortDesc,
        desc: currDoc.desc,
        benefits: currDoc.benefits,
        funding: currDoc.funding,
        timeline: currDoc.timeline,
        link: currDoc.link,
      }
      // for old data
      await this.model.update({_id: id}, {$set: { ...doc, editHistory: [emptyDoc, firstHistoryItem, doc] }})
    }

    return await this.show({ id })
  }

  public async list(param: any): Promise<Object> {
    const query = _.omit(param, ['results', 'page', 'sortBy', 'sortOrder', 'filter', 'profileListFor', 'search', 'tagsIncluded', 'referenceStatus'])
    const { sortBy, sortOrder, tagsIncluded, referenceStatus } = param



    if (!_.isEmpty(tagsIncluded)) {
      query['tags.type'] = { $in: tagsIncluded.split(',') }
    } else {
      query['tags.type'] = { $nin: listExlucdedFields }
    }

    if (referenceStatus === 'true') {
      query.reference = {$exists: true, $ne: []}
    } else {
      query.$or = [{reference: {$exists: false}}, {reference: {$eq: []}}]
    }

    const cursor = this.model.getDBInstance()
      .find(query)
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
      .populate('reference', constant.DB_SELECTED_FIELDS.CVOTE.ID_STATUS)
    const totalCursor = this.model.getDBInstance().find(query).count()

    if (sortBy) {
      const sortObject = {}
      sortObject[sortBy] = _.get(constant.SORT_ORDER, sortOrder, constant.SORT_ORDER.DESC)
      cursor.sort(sortObject)
    }

    if (param.results) {
      const results = parseInt(param.results, 10)
      const page = parseInt(param.page, 10)
      cursor.skip(results * (page - 1)).limit(results)
    }

    const list = await cursor
    const total = await totalCursor

    return {
      list,
      total,
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
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
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
  }

  private async notifyOwner(suggestionId: String, desc: String) {
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
  }

  public async addTag(param: any): Promise<Document> {
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

  /**
   * Admin only
   */
  public async archive(param: any): Promise<Document> {

    const { id: _id } = param
    const updateObject = {
      status: constant.SUGGESTION_STATUS.ARCHIVED,
    }
    await this.model.findOneAndUpdate({ _id }, updateObject)
    return this.model.findById(_id)
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
