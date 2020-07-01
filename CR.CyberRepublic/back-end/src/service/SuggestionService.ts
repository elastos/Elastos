import Base from './Base'
import * as _ from 'lodash'
import { Document, Types } from 'mongoose'
import * as jwt from 'jsonwebtoken'
import { constant } from '../constant'
import {
  validate,
  mail,
  user as userUtil,
  timestamp,
  permissions,
  logger,
  getDidPublicKey,
  utilCrypto
} from '../utility'
const Big = require('big.js')

const ObjectId = Types.ObjectId
const BASE_FIELDS = [
  'title',
  'type',
  'abstract',
  'goal',
  'motivation',
  'relevance',
  'budget',
  'budgetAmount',
  'elaAddress',
  'plan'
]

interface BudgetItem {
  type: string
  stage: string
  milestoneKey: string
  amount: string
  reasons: string
  criteria: string
}

export default class extends Base {
  private model: any
  private draftModel: any
  protected init() {
    this.model = this.getDBModel('Suggestion')
    this.draftModel = this.getDBModel('SuggestionDraft')
  }

  public async create(param: any): Promise<Document> {
    const doc = {
      ...param,
      version: 10,
      createdBy: _.get(this.currentUser, '_id'),
      contentType: constant.CONTENT_TYPE.MARKDOWN,
      // this is a hack for now, we should really be using aggregate pipeline + projection
      // in the sort query
      descUpdatedAt: new Date()
    }
    // save the document
    const result = await this.model.save(doc)
    await this.getDBModel('Suggestion_Edit_History').save({
      ...param,
      version: 10,
      suggestion: result._id
    })

    return result
  }

  // obsolete method
  public async sendMentionEmails(suggestion, mentions) {
    const db_user = this.getDBModel('User')
    const query = { role: constant.USER_ROLE.COUNCIL }
    const councilMembers = await db_user
      .getDBInstance()
      .find(query)
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
      _.map(councilMembers, (user) => {
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

  public async fixHistoryVersion(id: any) {
    const model = this.getDBModel('Suggestion_Edit_History')
    const list = await model
      .getDBInstance()
      .find({ suggestion: id })
      .sort({ createdAt: 1 })
    for (let i = 0, ver = 10; i < list.length; i++, ver += 1) {
      const _id = list[i]._id
      await model.getDBInstance().update({ _id }, { $set: { version: ver } })
    }
    const detail = await this.model.getDBInstance().findOne({ _id: id })
    if (detail) {
      if (!detail.version || detail.version < 10) {
        await this.model.update({ _id: id }, { $set: { version: 10 } })
      }
    }
  }

  public async saveHistoryGetCurrentVersion(id: any, doc: any) {
    const hisdoc = {
      ...doc,
      _id: undefined,
      createdAt: undefined,
      updatedAt: undefined,
      suggestion: id
    }
    const hismodel = this.getDBModel('Suggestion_Edit_History')
    const hisres = await hismodel.save(hisdoc)
    await this.fixHistoryVersion(id)
    const curhis = await hismodel.getDBInstance().findOne({ _id: hisres._id })
    return curhis.version
  }

  public async saveDraft(param: any): Promise<Document> {
    const { id, update } = param
    const userId = _.get(this.currentUser, '_id')
    const currDoc = await this.model.getDBInstance().findById(id)

    if (!currDoc) {
      throw 'Current document does not exist'
    }

    if (
      !userId.equals(_.get(currDoc, 'createdBy')) &&
      !permissions.isAdmin(_.get(this.currentUser, 'role'))
    ) {
      throw 'Only owner can edit suggestion'
    }

    const doc = _.pick(param, BASE_FIELDS)
    doc._id = ObjectId(id)
    doc.createdBy = ObjectId(userId)

    const currDraft = await this.draftModel.getDBInstance().findById(id)
    if (currDraft) {
      await this.draftModel.remove({ _id: ObjectId(id) })
    }

    doc.descUpdatedAt = new Date()

    let result = null
    if (update) {
      doc.version = await this.saveHistoryGetCurrentVersion(id, currDoc._doc)
      result = await this.draftModel.save(doc)
    } else {
      result = await this.draftModel.save(doc)
    }

    return result
  }

  public async update(param: any): Promise<Document> {
    const { id, update } = param
    const userId = _.get(this.currentUser, '_id')
    const currDoc = await this.model.getDBInstance().findById(id)

    if (!currDoc) {
      throw 'Current document does not exist'
    }

    if (_.get(currDoc, 'signature.data')) {
      throw 'Current document does not allow to edit'
    }

    if (
      !userId.equals(_.get(currDoc, 'createdBy')) &&
      !permissions.isAdmin(_.get(this.currentUser, 'role'))
    ) {
      throw 'Only owner can edit suggestion'
    }

    const currDraft = await this.draftModel.getDBInstance().findById(id)
    if (currDraft) {
      await this.draftModel.remove({ _id: ObjectId(id) })
    }

    const doc = _.pick(param, BASE_FIELDS)
    doc.descUpdatedAt = new Date()

    if (update) {
      doc.version = await this.saveHistoryGetCurrentVersion(id, doc)
      await this.model.update({ _id: id }, { $set: doc })
    } else {
      await this.model.update({ _id: id }, { $set: doc })
    }
    return this.show({ id })
  }

  public async list(param: any): Promise<Object> {
    const query = _.omit(param, [
      'results',
      'page',
      'sortBy',
      'sortOrder',
      'filter',
      'profileListFor',
      'search',
      'tagsIncluded',
      'referenceStatus',
      'status',
      'startDate',
      'endDate',
      'author',
      'budgetLow',
      'budgetHigh',
      'old'
    ])

    const {
      sortBy,
      sortOrder,
      tagsIncluded,
      referenceStatus,
      profileListFor
    } = param

    if (!profileListFor) {
      query.$or = []
      const search = _.trim(param.search)
      const filter = param.filter
      if (search && filter) {
        const SEARCH_FILTERS = {
          TITLE: 'TITLE',
          NUMBER: 'NUMBER',
          ABSTRACT: 'ABSTRACT',
          EMAIL: 'EMAIL',
          NAME: 'NAME'
        }

        if (filter === SEARCH_FILTERS.NUMBER) {
          query.$or = [{ displayId: parseInt(search) || 0 }]
        }

        if (filter === SEARCH_FILTERS.TITLE) {
          query.$or = [{ title: { $regex: search, $options: 'i' } }]
        }

        if (filter === SEARCH_FILTERS.ABSTRACT) {
          query.$or = [{ abstract: { $regex: search, $options: 'i' } }]
        }

        if (filter === SEARCH_FILTERS.EMAIL) {
          const db_user = this.getDBModel('User')
          const users = await db_user
            .getDBInstance()
            .find({
              $or: [{ email: { $regex: search, $options: 'i' } }]
            })
            .select('_id')
          const userIds = _.map(users, (el: { _id: string }) => el._id)
          query.$or = [{ createdBy: { $in: userIds } }]
        }

        if (filter === SEARCH_FILTERS.NAME) {
          const db_user = this.getDBModel('User')
          const pattern = search.split(' ').join('|')
          const users = await db_user
            .getDBInstance()
            .find({
              $or: [
                { username: { $regex: search, $options: 'i' } },
                { 'profile.firstName': { $regex: pattern, $options: 'i' } },
                { 'profile.lastName': { $regex: pattern, $options: 'i' } }
              ]
            })
            .select('_id')
          const userIds = _.map(users, (el: { _id: string }) => el._id)
          query.$or = [{ createdBy: { $in: userIds } }]
        }
      }

      let qryTagsType: any
      if (!_.isEmpty(tagsIncluded)) {
        qryTagsType = { $in: tagsIncluded.split(',') }
        query.$or.push({ 'tags.type': qryTagsType })
      }
      if (referenceStatus === 'true') {
        // if we have another tag selected we only want that tag and referenced suggestions
        query.$or.push({ reference: { $exists: true, $ne: [] } })
      }

      if (_.isEmpty(query.$or)) delete query.$or
      delete query['tags.type']
    }

    // status
    if (param.status && constant.SUGGESTION_STATUS[param.status]) {
      query.status = param.status
    }

    // old data
    if (param.old) {
      query.old = param.old
    }

    if (!param.old) {
      query.old = { $exists: false }
    }

    // budget
    if (param.budgetLow || param.budgetHigh) {
      query.budgetAmount = {}
      if (param.budgetLow && param.budgetLow.length) {
        query.budgetAmount['$gte'] = param.budgetLow
      }
      if (param.budgetHigh && param.budgetHigh.length) {
        query.budgetAmount['$lte'] = param.budgetHigh
      }
    }
    // isProposed
    if (param.isProposed) {
      query['reference.1'] = {
        $exists: true
      }
    }
    // startDate <  endDate
    if (
      param.startDate &&
      param.startDate.length &&
      param.endDate &&
      param.endDate.length
    ) {
      let endDate = new Date(param.endDate)
      endDate.setDate(endDate.getDate() + 1)
      query.createdAt = {
        $gte: new Date(param.startDate),
        $lte: endDate
      }
    }

    // console.log("[Author]" + param.author)

    // author
    if (param.author && param.author.length) {
      let search = param.author
      const db_user = this.getDBModel('User')
      const pattern = search.split(' ').join('|')
      const users = await db_user
        .getDBInstance()
        .find({
          $or: [
            { username: { $regex: search, $options: 'i' } },
            { 'profile.firstName': { $regex: pattern, $options: 'i' } },
            { 'profile.lastName': { $regex: pattern, $options: 'i' } }
          ]
        })
        .select('_id')

      const userIds = _.map(users, (el: { _id: string }) => el._id)

      // console.log("[Author.IDS]" + userIds)

      query.createdBy = { $in: userIds }
      // console.log("[Query]" + JSON.stringify(query))
    }
    // type
    if (
      param.type &&
      _.indexOf(_.values(constant.SUGGESTION_TYPE), param.type)
    ) {
      query.type = param.type
    }

    let cursor: any
    // suggestions on suggestion list page
    if (sortBy) {
      const sortObject = {}
      // hack to prioritize descUpdatedAt if it's createdAt
      if (sortBy === 'createdAt') {
        sortObject['descUpdatedAt'] = _.get(
          constant.SORT_ORDER,
          sortOrder,
          constant.SORT_ORDER.DESC
        )
      }
      sortObject[sortBy] = _.get(
        constant.SORT_ORDER,
        sortOrder,
        constant.SORT_ORDER.DESC
      )

      const excludedFields = [
        '-comments',
        '-goal',
        '-motivation',
        '-relevance',
        '-budget',
        '-plan',
        '-subscribers',
        '-likes',
        '-dislikes',
        '-updatedAt'
      ]

      cursor = this.model
        .getDBInstance()
        .find(query, excludedFields.join(' '))
        .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
        .populate('reference', constant.DB_SELECTED_FIELDS.CVOTE.ID_STATUS)
        .sort(sortObject)
    } else {
      // my suggestions on profile page
      cursor = this.model
        .getDBInstance()
        .find(
          query,
          'title activeness commentsNum createdAt dislikesNum displayId likesNum'
        )
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

  public async export2csv(param: any): Promise<Object> {
    const query = _.omit(param, [
      'results',
      'page',
      'sortBy',
      'sortOrder',
      'filter',
      'profileListFor',
      'search',
      'tagsIncluded',
      'referenceStatus'
    ])
    const {
      sortBy,
      sortOrder,
      tagsIncluded,
      referenceStatus,
      profileListFor
    } = param

    if (!profileListFor) {
      query.$or = []
      const search = _.trim(param.search)
      const filter = param.filter
      if (search && filter) {
        const SEARCH_FILTERS = {
          TITLE: 'TITLE',
          NUMBER: 'NUMBER',
          ABSTRACT: 'ABSTRACT',
          EMAIL: 'EMAIL',
          NAME: 'NAME'
        }

        if (filter === SEARCH_FILTERS.NUMBER) {
          query.$or = [{ displayId: parseInt(search) || 0 }]
        }

        if (filter === SEARCH_FILTERS.TITLE) {
          query.$or = [{ title: { $regex: search, $options: 'i' } }]
        }

        if (filter === SEARCH_FILTERS.ABSTRACT) {
          query.$or = [{ abstract: { $regex: search, $options: 'i' } }]
        }

        if (filter === SEARCH_FILTERS.EMAIL) {
          const db_user = this.getDBModel('User')
          const users = await db_user
            .getDBInstance()
            .find({
              $or: [{ email: { $regex: search, $options: 'i' } }]
            })
            .select('_id')
          const userIds = _.map(users, (el: { _id: string }) => el._id)
          query.$or = [{ createdBy: { $in: userIds } }]
        }

        if (filter === SEARCH_FILTERS.NAME) {
          const db_user = this.getDBModel('User')
          const pattern = search.split(' ').join('|')
          const users = await db_user
            .getDBInstance()
            .find({
              $or: [
                { username: { $regex: search, $options: 'i' } },
                { 'profile.firstName': { $regex: pattern, $options: 'i' } },
                { 'profile.lastName': { $regex: pattern, $options: 'i' } }
              ]
            })
            .select('_id')
          const userIds = _.map(users, (el: { _id: string }) => el._id)
          query.$or = [{ createdBy: { $in: userIds } }]
        }
      }

      let qryTagsType: any
      if (!_.isEmpty(tagsIncluded)) {
        qryTagsType = { $in: tagsIncluded.split(',') }
        query.$or.push({ 'tags.type': qryTagsType })
      }
      if (referenceStatus === 'true') {
        // if we have another tag selected we only want that tag and referenced suggestions
        query.$or.push({ reference: { $exists: true, $ne: [] } })
      }

      if (_.isEmpty(query.$or)) delete query.$or
      delete query['tags.type']
    }

    let cursor: any
    // suggestions on suggestion list page
    if (sortBy) {
      const sortObject = {}
      // hack to prioritize descUpdatedAt if it's createdAt
      if (sortBy === 'createdAt') {
        sortObject['descUpdatedAt'] = _.get(
          constant.SORT_ORDER,
          sortOrder,
          constant.SORT_ORDER.DESC
        )
      }
      sortObject[sortBy] = _.get(
        constant.SORT_ORDER,
        sortOrder,
        constant.SORT_ORDER.DESC
      )

      const excludedFields = [
        '-comments',
        '-goal',
        '-motivation',
        '-relevance',
        '-budget',
        '-plan',
        '-subscribers',
        '-likes',
        '-dislikes',
        '-updatedAt'
      ]

      cursor = this.model
        .getDBInstance()
        .find(query /*, excludedFields.join(' ')*/)
        .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
        .populate('reference', constant.DB_SELECTED_FIELDS.CVOTE.ID_STATUS)
        .sort(sortObject)
    } else {
      // my suggestions on profile page
      cursor = this.model
        .getDBInstance()
        .find(
          query /*, 'title activeness commentsNum createdAt dislikesNum displayId likesNum'*/
        )
        .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
        .populate('reference', constant.DB_SELECTED_FIELDS.CVOTE.ID_STATUS)
    }

    /*if (param.results) {
       const results = parseInt(param.results, 10)
       const page = parseInt(param.page, 10)
       cursor.skip(results * (page - 1)).limit(results)
       }*/

    // status
    if (param.status && constant.SUGGESTION_STATUS[param.status]) {
      query.status = param.status
    }
    // budget
    if (param.budgetLow || param.budgetHigh) {
      query.budgetAmount = {}
      if (param.budgetLow && param.budgetLow.length) {
        query.budgetAmount['$gte'] = param.budgetLow
      }
      if (param.budgetHigh && param.budgetHigh.length) {
        query.budgetAmount['$lte'] = param.budgetHigh
      }
    }
    // isProposed
    if (param.isProposed) {
      query['reference.1'] = {
        $exists: true
      }
    }
    // startDate <  endDate
    if (
      param.startDate &&
      param.startDate.length &&
      param.endDate &&
      param.endDate.length
    ) {
      let endDate = new Date(param.endDate)
      endDate.setDate(endDate.getDate() + 1)
      query.createdAt = {
        $gte: new Date(param.startDate),
        $lte: endDate
      }
    }

    // author
    if (param.author && param.author.length) {
      let search = param.author
      const db_user = this.getDBModel('User')
      const pattern = search.split(' ').join('|')
      const users = await db_user
        .getDBInstance()
        .find({
          $or: [
            { username: { $regex: search, $options: 'i' } },
            { 'profile.firstName': { $regex: pattern, $options: 'i' } },
            { 'profile.lastName': { $regex: pattern, $options: 'i' } }
          ]
        })
        .select('_id')
      const userIds = _.map(users, (el: { _id: string }) => el._id)
      query.createdBy = { $in: userIds }
    }
    // type
    if (
      param.type &&
      _.indexOf(_.values(constant.SUGGESTION_TYPE), param.type)
    ) {
      query.type = param.type
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

  public async showInModel(model: any, param: any): Promise<any> {
    const { id: _id, incViewsNum } = param
    // access suggestion info by reference number
    const isNumber = /^\d*$/.test(_id)
    let query: any
    if (isNumber) {
      query = { displayId: parseInt(_id) }
    } else {
      query = { _id }
    }

    if (incViewsNum === 'true') {
      await model.findOneAndUpdate(query, {
        $inc: { viewsNum: 1, activeness: 1 }
      })
    }

    let doc = await model
      .getDBInstance()
      .findOne(query)
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
      .populate('reference', constant.DB_SELECTED_FIELDS.CVOTE.ID_STATUS)

    if (!doc) {
      return { success: true, empty: true }
    }

    // proposed by council
    const db_cvote = this.getDBModel('CVote')

    const cvoteList = await db_cvote
      .getDBInstance()
      .findOne({ reference: { $all: [doc._id] } })
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

    doc = JSON.parse(JSON.stringify(doc))
    if (cvoteList) {
      doc.proposer = cvoteList.createdBy
    }

    // deal with 7e-08
    if (doc.budgetAmount) {
      doc.budgetAmount = Big(doc.budgetAmount).toFixed()
    }

    if (doc && _.isEmpty(doc.comments)) return doc

    if (doc && doc.comments) {
      for (const comment of doc.comments) {
        for (const thread of comment) {
          await model.getDBInstance().populate(thread, {
            path: 'createdBy',
            select: `${constant.DB_SELECTED_FIELDS.USER.NAME} profile.avatar`
          })
        }
      }
    }

    if (!doc.version || doc.version < 10) doc.version = 10
    return doc
  }
  public async show(param: any): Promise<any> {
    return this.showInModel(this.model, param)
  }
  public async showDraft(param: any): Promise<any> {
    return this.showInModel(this.draftModel, param)
  }

  public async editHistories(param: any): Promise<Document[]> {
    await this.fixHistoryVersion(param.id)
    return await this.getDBModel('Suggestion_Edit_History')
      .getDBInstance()
      .find({ suggestion: param.id })
      .sort({ version: -1 })
  }

  public async revertVersion(param: any): Promise<any> {
    const { id, version } = param
    const userId = _.get(this.currentUser, '_id')
    const currDoc = await this.model.getDBInstance().findById(id)

    if (!currDoc) {
      throw 'Current document does not exist'
    }

    if (
      !userId.equals(_.get(currDoc, 'createdBy')) &&
      !permissions.isAdmin(_.get(this.currentUser, 'role'))
    ) {
      throw 'Only owner can edit suggestion'
    }

    const currVer = await this.model.getDBInstance().findOne({ _id: id })

    if (_.get(currVer, 'signature.data')) {
      throw 'This suggestion had been signed.'
    }

    const rVer = await this.getDBModel('Suggestion_Edit_History')
      .getDBInstance()
      .findOne({ suggestion: id, version })
    if (!currVer || !rVer) {
      throw 'Current document does not exist'
    }

    Object.assign(currVer, _.pick(rVer, BASE_FIELDS))
    currVer.version = rVer.version

    await this.model.update({ _id: id }, { $set: currVer })

    return { id, version }
  }

  // like or unlike
  public async like(param: any): Promise<Document> {
    const { id: _id } = param
    const userId = _.get(this.currentUser, '_id')
    const doc = await this.model.findById(_id)
    const { likes, dislikes } = doc

    // can not both like and dislike, use ObjectId.equals to compare
    if (_.findIndex(dislikes, (oid) => userId.equals(oid)) !== -1) return doc

    // already liked, will unlike, use ObjectId.equals to compare
    if (_.findIndex(likes, (oid) => userId.equals(oid)) !== -1) {
      await this.model.findOneAndUpdate(
        { _id },
        {
          $pull: { likes: userId },
          $inc: { likesNum: -1, activeness: -1 }
        }
      )
    } else {
      // not like yet, will like it
      await this.model.findOneAndUpdate(
        { _id },
        {
          $push: { likes: userId },
          $inc: { likesNum: 1, activeness: 1 }
        }
      )
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
    if (_.findIndex(likes, (oid) => userId.equals(oid)) !== -1) return doc

    // already liked, will unlike, use ObjectId.equals to compare
    if (_.findIndex(dislikes, (oid) => userId.equals(oid)) !== -1) {
      await this.model.findOneAndUpdate(
        { _id },
        {
          $pull: { dislikes: userId },
          $inc: { dislikesNum: -1, activeness: -1 }
        }
      )
    } else {
      // not like yet, will like it
      await this.model.findOneAndUpdate(
        { _id },
        {
          $push: { dislikes: userId },
          $inc: { dislikesNum: 1, activeness: 1 }
        }
      )
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
      const db_user = this.getDBModel('User')
      const currentUserId = _.get(this.currentUser, '_id')
      const councilMember = await db_user.findById(currentUserId)
      const suggestion = await this.model
        .getDBInstance()
        .findById(suggestionId)
        .populate(
          'subscribers.user',
          constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL
        )
        .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)

      // get users: creator and subscribers
      const toUsers = _.map(suggestion.subscribers, 'user') || []
      toUsers.push(suggestion.createdBy)
      const toMails = _.map(toUsers, 'email')

      const subject = 'The suggestion is under consideration of Council.'
      const body = `
      <p>Council member ${userUtil.formatUsername(
        councilMember
      )} has marked this suggestion ${
        suggestion.title
      } as "Under Consideration"</p>
      <br />
      <p>Click this link to view more details: <a href="${
        process.env.SERVER_URL
      }/suggestion/${suggestion._id}">${process.env.SERVER_URL}/suggestion/${
        suggestion._id
      }</a></p>
      <br /> <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
      `

      const recVariables = _.zipObject(
        toMails,
        _.map(toUsers, (user) => {
          return {
            _id: user._id,
            username: userUtil.formatUsername(user)
          }
        })
      )

      const mailObj = {
        to: toMails,
        // toName: ownerToName,
        subject,
        body,
        recVariables
      }

      mail.send(mailObj)
    } catch (error) {
      logger.error(error)
    }
  }

  private async notifyOwner(suggestionId: String, desc: String) {
    try {
      const db_user = this.getDBModel('User')
      const currentUserId = _.get(this.currentUser, '_id')
      const councilMember = await db_user.findById(currentUserId)
      const suggestion = await this.model
        .getDBInstance()
        .findById(suggestionId)
        .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)

      // get users: creator and subscribers
      const toUsers = [suggestion.createdBy]
      const toMails = _.map(toUsers, 'email')

      const subject = 'Your suggestion needs more info for Council.'
      const body = `
        <p>Council member ${userUtil.formatUsername(
          councilMember
        )} has marked your suggestion ${
        suggestion.title
      } as "Need more information".</p>
        <br />
        <p>"${desc}"</p>
        <br />
        <p>Click this link to view more details: <a href="${
          process.env.SERVER_URL
        }/suggestion/${suggestion._id}">${process.env.SERVER_URL}/suggestion/${
        suggestion._id
      }</a></p>
        <br /> <br />
        <p>Thanks</p>
        <p>Cyber Republic</p>
      `

      const recVariables = _.zipObject(
        toMails,
        _.map(toUsers, (user) => {
          return {
            _id: user._id,
            username: userUtil.formatUsername(user)
          }
        })
      )

      const mailObj = {
        to: toMails,
        // toName: ownerToName,
        subject,
        body,
        recVariables
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

      if (
        _.findIndex(currDoc.tags, (tagObj: any) => tagObj.type === type) !== -1
      )
        return currDoc

      const tag: any = {
        type,
        createdBy: _.get(this.currentUser, '_id')
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
    } catch (error) {
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
    const subject = `Need due diligence on suggestion #${sugg.displayId}`
    const body = `
      <p>Council member ${council} requested secretary to do due diligence on suggestion #${sugg.displayId}</p>
      <br />
      <p>Click the link to view the suggestion detail: <a href="${process.env.SERVER_URL}/suggestion/${sugg._id}">${process.env.SERVER_URL}/suggestion/${sugg._id}</a></p>
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
    const subject = `Need advisory on suggestion #${sugg.displayId}`
    const body = `
      <p>Council member ${council} requested secretary to provide advisory on suggestion #${sugg.displayId}</p>
      <br />
      <p>Click the link to view the suggestion detail: <a href="${process.env.SERVER_URL}/suggestion/${sugg._id}">${process.env.SERVER_URL}/suggestion/${sugg._id}</a></p>
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
      (user) => !user._id.equals(currentUserId)
    )
    const toMails = _.map(toUsers, 'email')
    const recVariables = _.zipObject(
      toMails,
      _.map(toUsers, (user) => {
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
    const { id: _id, isArchived } = param
    const suggestion = await this.model
      .getDBInstance()
      .findById(_id)
      .populate('createdBy')
    if (!suggestion) {
      return
    }
    const isAdmin = this.currentUser.role === constant.USER_ROLE.ADMIN
    const isAuthor = suggestion.createdBy._id.equals(this.currentUser._id)
    if (!(isAdmin || isAuthor)) {
      return
    }
    let field = {}
    if (isArchived && isArchived === true) {
      field = {
        status: constant.SUGGESTION_STATUS.ACTIVE
      }
    } else {
      field = {
        status: constant.SUGGESTION_STATUS.ARCHIVED
      }
    }
    try {
      await this.model.update({ _id }, field)
      return { success: true, message: 'ok' }
    } catch (err) {
      return { success: false, message: 'ok' }
    }
  }

  public async delete(param: any): Promise<Document> {
    const { id: _id } = param
    return this.model.findByIdAndDelete(_id)
  }

  /**
   * Wallet Api
   */
  public async getSuggestion(id): Promise<any> {
    const fileds = ['_id', 'displayId', 'title', 'abstract', 'createdAt']

    const suggestion = await this.model
      .getDBInstance()
      .findOne({ _id: id }, fileds.join(' '))
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

    if (!suggestion) {
      return {
        code: 400,
        message: 'Invalid request parameters',
        // tslint:disable-next-line:no-null-keyword
        data: null
      }
    }

    const createdBy = suggestion.createdBy
    const address = `${process.env.SERVER_URL}/suggestion/${suggestion._id}`
    const did = _.get(createdBy, 'did.id')
    const didName = _.get(createdBy, 'did.didName')
    const result = _.omit(suggestion._doc, [
      '_id',
      'id',
      'displayId',
      'createdBy',
      'abstract'
    ])

    return {
      ...result,
      createdAt: timestamp.second(result.createdAt),
      id: suggestion.displayId,
      abs: suggestion.abstract,
      address,
      did,
      didName
    }
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

  private convertBudget(budget: [BudgetItem]) {
    const chainBudgetType = {
      ADVANCE: 'Imprest',
      CONDITIONED: 'NormalPayment',
      COMPLETION: 'FinalPayment'
    }
    const initiation = _.find(budget, ['type', 'ADVANCE'])
    const budgets = budget.map((item: BudgetItem) => {
      const stage = parseInt(item.milestoneKey, 10)
      return {
        type: chainBudgetType[item.type],
        stage: initiation ? stage : stage + 1,
        amount: Big(`${item.amount}e+8`).toString()
      }
    })
    return budgets
  }

  private getDraftHash(suggestion: any) {
    const fields = [
      '_id',
      'title',
      'type',
      'abstract',
      'motivation',
      'goal',
      'plan',
      'relevance',
      'budget',
      'budgetAmount',
      'elaAddress'
    ]
    const content = {}
    const sortedFields = _.sortBy(fields)
    for (let index in sortedFields) {
      const field = sortedFields[index]
      content[field] = suggestion[field]
    }
    return utilCrypto.sha256D(JSON.stringify(content))
  }

  /* author signs a suggestion */
  public async getSignatureUrl(param: { id: string }) {
    try {
      const { id } = param
      const suggestion = await this.model
        .getDBInstance()
        .findById(id)
        .populate('createdBy')

      if (!suggestion) {
        return { success: false, message: 'No this suggestion' }
      }

      // check if current user is the owner of this suggestion
      if (!suggestion.createdBy._id.equals(this.currentUser._id)) {
        return { success: false, message: 'You are not the owner' }
      }

      if (_.get(suggestion, 'signature.data')) {
        return { success: false, message: 'You had signed.' }
      }

      const did = _.get(this.currentUser, 'did.id')
      if (!did) {
        return { success: false, message: 'Your DID not bound.' }
      }

      const rs: {
        compressedPublicKey: string
        publicKey: string
      } = await getDidPublicKey(did)
      if (!rs) {
        return {
          success: false,
          message: `Can not get your did's public key.`
        }
      }

      let ownerPublicKey = rs.compressedPublicKey
      const draftHash = this.getDraftHash(suggestion)
      await this.model.update(
        { _id: suggestion._id },
        { $set: { draftHash, ownerPublicKey } }
      )

      const now = Math.floor(Date.now() / 1000)
      const jwtClaims = {
        iat: now,
        exp: now + 60 * 60 * 24,
        command: 'createsuggestion',
        iss: process.env.APP_DID,
        sid: suggestion._id,
        callbackurl: `${process.env.API_URL}/api/suggestion/signature-callback`,
        data: {
          proposaltype: 'normal',
          categorydata: '',
          ownerpublickey: ownerPublicKey,
          drafthash: draftHash,
          budgets: this.convertBudget(suggestion.budget),
          recipient: suggestion.elaAddress
        }
      }

      const jwtToken = jwt.sign(
        JSON.stringify(jwtClaims),
        process.env.APP_PRIVATE_KEY,
        {
          algorithm: 'ES256'
        }
      )
      const url = `elastos://crproposal/${jwtToken}`
      return { success: true, url }
    } catch (err) {
      logger.error(err)
      return { success: false }
    }
  }

  public async signatureCallback(param: any) {
    try {
      const jwtToken = param.jwt
      const claims: any = jwt.decode(jwtToken)
      if (!_.get(claims, 'req')) {
        return {
          code: 400,
          success: false,
          message: 'Problems parsing jwt token.'
        }
      }

      const payload: any = jwt.decode(
        claims.req.slice('elastos://crproposal/'.length)
      )
      if (!_.get(payload, 'sid')) {
        return {
          code: 400,
          success: false,
          message: 'Problems parsing jwt token of CR website.'
        }
      }

      const suggestion = await this.model.findById({
        _id: payload.sid
      })
      if (!suggestion) {
        return {
          code: 400,
          success: false,
          message: 'There is no this suggestion.'
        }
      }
      const signature = _.get(suggestion, 'signature.data')
      if (signature) {
        return {
          code: 400,
          success: false,
          message: 'This suggestion had been signed.'
        }
      }
      const rs: any = await getDidPublicKey(claims.iss)
      if (!rs) {
        return {
          code: 400,
          success: false,
          message: `Can not get your did's public key.`
        }
      }

      // verify response data from ela wallet
      return jwt.verify(
        jwtToken,
        rs.publicKey,
        async (err: any, decoded: any) => {
          if (err) {
            await this.model.update(
              { _id: payload.sid },
              {
                $set: {
                  signature: { message: 'Verify signatrue failed.' }
                }
              }
            )
            return {
              code: 401,
              success: false,
              message: 'Verify signatrue failed.'
            }
          } else {
            try {
              await this.model.update(
                { _id: payload.sid },
                { $set: { signature: { data: decoded.data } } }
              )
              return { code: 200, success: true, message: 'Ok' }
            } catch (err) {
              logger.error(err)
              return {
                code: 500,
                success: false,
                message: 'Something went wrong'
              }
            }
          }
        }
      )
    } catch (err) {
      logger.error(err)
      return {
        code: 500,
        success: false,
        message: 'Something went wrong'
      }
    }
  }

  public async checkSignature(param: any) {
    const { id } = param
    const suggestion = await this.show({ id })
    if (suggestion) {
      const signature = _.get(suggestion, 'signature.data')
      if (signature) {
        return { success: true, data: suggestion }
      }
      const message = _.get(suggestion, 'signature.message')
      if (message) {
        // clear error message
        await this.model.update({ _id: id }, { $unset: { signature: true } })
        return { success: false, message }
      }
    } else {
      return { success: false }
    }
  }
  /* end */

  // a council member signs a suggestion
  public async getCMSignatureUrl(param: { id: string }) {
    try {
      const councilMemberDid = _.get(this.currentUser, 'did.id')
      if (!councilMemberDid) {
        return { success: false, message: 'Your DID not bound.' }
      }

      const role = _.get(this.currentUser, 'role')
      if (!permissions.isCouncil(role)) {
        return { success: false, message: 'No access right.' }
      }

      const { id } = param
      const suggestion = await this.model.findById(id)
      if (!suggestion) {
        return { success: false, message: 'No this suggestion.' }
      }
      if (suggestion && !_.isEmpty(suggestion.reference)) {
        return {
          success: false,
          message: 'This suggestion had been made into a proposal.'
        }
      }
      if (suggestion && suggestion.proposed === true) {
        return {
          success: false,
          message: 'This suggestion is proposed.'
        }
      }
      const currDate = Date.now()
      const now = Math.floor(currDate / 1000)
      const jwtClaims = {
        iat: now,
        exp: now + 60 * 60 * 24,
        command: 'createproposal',
        iss: process.env.APP_DID,
        sid: suggestion._id,
        data: {
          proposaltype: 'normal',
          categorydata: '',
          ownerpublickey: suggestion.ownerPublicKey,
          drafthash: suggestion.draftHash,
          budgets: this.convertBudget(suggestion.budget),
          recipient: suggestion.elaAddress,
          signature: suggestion.signature.data,
          did: councilMemberDid
        }
      }
      const jwtToken = jwt.sign(
        JSON.stringify(jwtClaims),
        process.env.APP_PRIVATE_KEY,
        {
          algorithm: 'ES256'
        }
      )
      await this.model.update(
        { _id: suggestion._id },
        { $push: { proposers: { did: councilMemberDid, timestamp: now } } }
      )
      const url = `elastos://crproposal/${jwtToken}`
      return { success: true, url }
    } catch (err) {
      logger.error(err)
      return { success: false }
    }
  }
}
