import Base from './Base'
import { Document } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../constant'
import { permissions } from '../utility'
import * as moment from 'moment'
import { mail, user as userUtil } from '../utility'

let tm = undefined

const restrictedFields = {
  update: [
    '_id',
    'createdBy',
    'createdAt',
    'proposedAt',
  ]
}

export default class extends Base {
  // create a DRAFT propoal with minimal info
  public async createDraft(param: any): Promise<Document> {
    const db_cvote = this.getDBModel('CVote')
    const {
      title, proposedBy, proposer,
    } = param

    const vid = await this.getNewVid()

    const doc: any = {
      title,
      vid,
      status: constant.CVOTE_STATUS.DRAFT,
      published: false,
      contentType: constant.CONTENT_TYPE.MARKDOWN,
      proposedBy,
      proposer,
      createdBy: this.currentUser._id
    }

    try {
      return await db_cvote.save(doc)
    } catch (error) {
      return
    }
  }

  /**
   *
   * @param param
   * @returns {Promise<"mongoose".Document>}
   */
  public async updateDraft(param: any): Promise<Document> {
    const db_cvote = this.getDBModel('CVote')
    const {
      _id, title, abstract, goal, motivation, relevance, budget, plan
    } = param

    if (!this.currentUser || !this.currentUser._id) {
      throw 'cvoteservice.update - invalid current user'
    }

    if (!this.canManageProposal()) {
      throw 'cvoteservice.update - not council'
    }

    const cur = await db_cvote.findOne({ _id })
    if (!cur) {
      throw 'cvoteservice.update - invalid proposal id'
    }

    const doc: any = {
      contentType: constant.CONTENT_TYPE.MARKDOWN,
    }

    if (title) doc.title = title
    if (abstract) doc.abstract = abstract
    if (goal) doc.goal = goal
    if (motivation) doc.motivation = motivation
    if (relevance) doc.relevance = relevance
    if (budget) doc.budget = budget
    if (plan) doc.plan = plan

    try {
      await db_cvote.update({ _id }, doc)
      const res = await this.getById(_id)
      return res
    } catch (error) {
      console.log('error happened: ', error)
      return
    }
  }

  public async create(param): Promise<Document> {
    const db_cvote = this.getDBModel('CVote')
    const db_user = this.getDBModel('User')
    const db_suggestion = this.getDBModel('Suggestion')
    const currentUserId = _.get(this.currentUser, '_id')
    const {
      title, published, proposedBy, proposer, motionId,
      suggestionId, abstract, goal, motivation, relevance, budget, plan
    } = param

    const vid = await this.getNewVid()
    const status = published ? constant.CVOTE_STATUS.PROPOSED : constant.CVOTE_STATUS.DRAFT

    const doc: any = {
      title,
      vid,
      status,
      published,
      contentType: constant.CONTENT_TYPE.MARKDOWN,
      proposedBy,
      abstract,
      goal,
      motivation,
      relevance,
      budget,
      plan,
      proposer,
      motionId,
      createdBy: this.currentUser._id
    }

    const suggestion = suggestionId && await db_suggestion.findById(suggestionId)
    if (!_.isEmpty(suggestion)) {
      doc.reference = suggestionId
    }

    const councilMembers = await db_user.find({ role: constant.USER_ROLE.COUNCIL })
    const voteResult = []
    if (published) {
      doc.proposedAt = Date.now()
      _.each(councilMembers, user => {
        // use ObjectId.equals
        const value = currentUserId.equals(user._id) ? constant.CVOTE_RESULT.SUPPORT : constant.CVOTE_RESULT.UNDECIDED
        voteResult.push({ votedBy: user._id, value })
      })
      doc.voteResult = voteResult
      doc.voteHistory = voteResult
    }

    try {
      const res = await db_cvote.save(doc)
      // add reference with suggestion
      if (!_.isEmpty(suggestion)) {
        await db_suggestion.update({ _id: suggestionId }, { $addToSet: { reference: res._id }})
        // notify creator and subscribers
        if (published) this.notifySubscribers(res)
      }

      // notify council member to vote
      if (published) this.notifyCouncil(res)

      return res
    } catch (error) {
      console.log('error happened: ', error)
      return
    }
  }

  private async notifySubscribers(cvote: any) {
    const db_suggestion = this.getDBModel('Suggestion')
    const suggestionId = _.get(cvote, 'reference')
    if (!suggestionId) return
    const suggestion = await db_suggestion.getDBInstance().findById(suggestionId)
    .populate('subscribers.user', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)
    .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)

    // get users: creator and subscribers
    const toUsers = _.map(suggestion.subscribers, 'user') || []
    toUsers.push(suggestion.createdBy)
    const toMails = _.map(toUsers, 'email')

    // compose email object
    const subject = `The suggestion is referred in Proposal #${cvote.vid}`
    const body = `
      <p>Council member ${cvote.proposedBy} has refer to your suggestion ${suggestion.title} in a proposal #${cvote.vid}.</p>
      <br />
      <p>Click this link to view more details:</p>
      <p><a href="${process.env.SERVER_URL}/proposals/${cvote._id}">${process.env.SERVER_URL}/proposals/${cvote._id}</a></p>
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

    // send email
    mail.send(mailObj)
  }

  private async notifyCouncil(cvote: any) {
    const db_user = this.getDBModel('User')
    const currentUserId = _.get(this.currentUser, '_id')
    const councilMembers = await db_user.find({ role: constant.USER_ROLE.COUNCIL })
    const toUsers = _.filter(councilMembers, user => !user._id.equals(currentUserId))
    const toMails = _.map(toUsers, 'email')

    const subject = `New Proposal: ${cvote.title}`
    const body = `
      <p>There is a new proposal added:</p>
      <br />
      <p>${cvote.title}</p>
      <br />
      <p>Click this link to view more details: <a href="${process.env.SERVER_URL}/proposals/${cvote._id}">${process.env.SERVER_URL}/proposals/${cvote._id}</a></p>
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

  private async notifyCouncilToVote() {
    // find cvote before 1 day expiration without vote yet for each council member
    const db_cvote = this.getDBModel('CVote')
    const nearExpiredTime = Date.now() - (constant.CVOTE_EXPIRATION - constant.ONE_DAY)
    const unvotedCVotes = await db_cvote.getDBInstance().find(
      {
        proposedAt: { $lt: nearExpiredTime, $gt: (Date.now() - constant.CVOTE_EXPIRATION) },
        notified: { $ne: true },
        status: constant.CVOTE_STATUS.PROPOSED
      }
    ).populate('voteResult.votedBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)

    _.each(unvotedCVotes, cvote => {
      _.each(cvote.voteResult, result => {
        if (result.value === constant.CVOTE_RESULT.UNDECIDED) {
          // send email to council member to notify to vote
          const { title, _id } = cvote
          const subject = `Proposal Vote Reminder: ${title}`
          const body = `
            <p>You only got 24 hours to vote this proposal:</p>
            <br />
            <p>${title}</p>
            <br />
            <p>Click this link to vote: <a href="${process.env.SERVER_URL}/proposals/${_id}">${process.env.SERVER_URL}/proposals/${_id}</a></p>
            <br /> <br />
            <p>Thanks</p>
            <p>Cyber Republic</p>
          `
          const mailObj = {
            to: result.votedBy.email,
            toName: userUtil.formatUsername(result.votedBy),
            subject,
            body,
          }
          mail.send(mailObj)

          // update notified to true
          db_cvote.update({ _id: cvote._id }, { $set: { notified: true }})
        }
      })
    })
  }

  /**
   * List proposals, only an admin may request and view private records
   *
   * We expect the front-end to always call with {published: true}
   *
   * TODO: what's the rest way of encoding multiple values for a field?
   *
   * Instead of magic params, we should have just different endpoints I think,
   * this method should be as dumb as possible
   *
   * @param query
   * @returns {Promise<"mongoose".Document>}
   */
  public async list(param): Promise<Document> {

    const db_cvote = this.getDBModel('CVote')
    const db_user = this.getDBModel('User')
    const currentUserId = _.get(this.currentUser, '_id')
    const userRole = _.get(this.currentUser, 'role')
    const query: any = {}

    if (!param.published) {
      if (!this.isLoggedIn() || !this.canManageProposal()) {
        throw 'cvoteservice.list - unpublished proposals only visible to council/secretary'
      } else if (param.voteResult === constant.CVOTE_RESULT.UNDECIDED && permissions.isCouncil(userRole)) {
        // get unvoted by current council
        query.voteResult = {
          $elemMatch: {
            value: constant.CVOTE_RESULT.UNDECIDED,
            votedBy: currentUserId
          }
        }
        query.published = true
        query.status = constant.CVOTE_STATUS.PROPOSED
      }
    } else {
      query.published = param.published
    }

    if (param.$or) query.$or = param.$or

    const list = await db_cvote.list(query, {
      vid: -1,
      // createdAt: -1
    }, 100)

    for (const item of list) {
      if (item.createdBy) {
        const u = await db_user.findOne({ _id: item.createdBy })
        if (!_.isEmpty) item.createdBy = u.username
      }
    }

    return list
  }

  /**
   *
   * @param param
   * @returns {Promise<"mongoose".Document>}
   */
  public async update(param): Promise<Document> {
    const db_user = this.getDBModel('User')
    const db_cvote = this.getDBModel('CVote')
    const currentUserId = _.get(this.currentUser, '_id')
    const {
      _id, published, notes, title,
      abstract, goal, motivation, relevance, budget, plan
    } = param

    if (!this.currentUser || !this.currentUser._id) {
      throw 'cvoteservice.update - invalid current user'
    }

    if (!this.canManageProposal()) {
      throw 'cvoteservice.update - not council'
    }

    const cur = await db_cvote.findOne({ _id })
    if (!cur) {
      throw 'cvoteservice.update - invalid proposal id'
    }

    const doc: any = {
      contentType: constant.CONTENT_TYPE.MARKDOWN,
    }
    const willChangeToPublish = published === true && cur.status === constant.CVOTE_STATUS.DRAFT

    if (title) doc.title = title
    if (abstract) doc.abstract = abstract
    if (goal) doc.goal = goal
    if (motivation) doc.motivation = motivation
    if (relevance) doc.relevance = relevance
    if (budget) doc.budget = budget
    if (plan) doc.plan = plan

    if (willChangeToPublish) {
      doc.status = constant.CVOTE_STATUS.PROPOSED
      doc.published = published
      doc.proposedAt = Date.now()
      const councilMembers = await db_user.find({ role: constant.USER_ROLE.COUNCIL })
      const voteResult = []
      _.each(councilMembers, user => {
        // use ObjectId.equals
        const value = currentUserId.equals(user._id) ? constant.CVOTE_RESULT.SUPPORT : constant.CVOTE_RESULT.UNDECIDED
        voteResult.push({ votedBy: user._id, value })
        // send email
      })
      doc.voteResult = voteResult
      doc.voteHistory = voteResult
    }

    // always allow secretary to edit notes
    if (notes) doc.notes = notes
    try {
      await db_cvote.update({ _id }, doc)
      const res = await this.getById(_id)
      if (willChangeToPublish) {
        this.notifyCouncil(res)
        this.notifySubscribers(res)
      }
      return res
    } catch (error) {
      console.log('error happened: ', error)
      return
    }
  }

  public async finishById(id): Promise<any> {
    const db_cvote = this.getDBModel('CVote')
    const cur = await db_cvote.findOne({ _id: id })
    if (!cur) {
      throw 'invalid proposal id'
    }
    if (!this.canManageProposal()) {
      throw 'cvoteservice.finishById - not council'
    }
    if (_.includes([constant.CVOTE_STATUS.FINAL], cur.status)) {
      throw 'proposal already completed.'
    }

    const rs = await db_cvote.update({ _id: id }, {
      $set: {
        status: constant.CVOTE_STATUS.FINAL
      }
    })

    return rs
  }

  public async getById(id): Promise<any> {
    const db_cvote = this.getDBModel('CVote')
    const rs = await db_cvote.getDBInstance().findOne({ _id: id })
      .populate('voteResult.votedBy', constant.DB_SELECTED_FIELDS.USER.NAME_AVATAR)
      .populate('proposer', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)
      .populate('reference', constant.DB_SELECTED_FIELDS.SUGGESTION.ID)
    return rs
  }

  public async getNewVid() {
    const db_cvote = this.getDBModel('CVote')
    const n = await db_cvote.count({})
    return n + 1
  }

  public isExpired(data: any, extraTime = 0): Boolean {
    const ct = moment(data.proposedAt || data.createdAt).valueOf()
    if (Date.now() - ct - extraTime > constant.CVOTE_EXPIRATION) {
      return true
    }
    return false
  }

  // proposal active/passed
  public isActive(data): Boolean {
    const supportNum = _.countBy(data.voteResult, 'value')[constant.CVOTE_RESULT.SUPPORT] || 0
    return supportNum > data.voteResult.length * 0.5
  }

  // proposal rejected
  public isRejected(data): Boolean {
    const rejectNum = _.countBy(data.voteResult, 'value')[constant.CVOTE_RESULT.REJECT] || 0
    return rejectNum > data.voteResult.length * 0.5
  }

  public async vote(param): Promise<Document> {
    const db_cvote = this.getDBModel('CVote')
    const { _id, value, reason } = param
    const cur = await db_cvote.findOne({ _id })
    const votedBy = _.get(this.currentUser, '_id')
    if (!cur) {
      throw 'invalid proposal id'
    }

    await db_cvote.update(
      {
        _id,
        'voteResult.votedBy': votedBy
      },
      {
        $set: {
          'voteResult.$.value': value,
          'voteResult.$.reason': reason || '',
        },
        $push: {
          voteHistory: {
            value,
            reason,
            votedBy,
          }
        }
      }
    )

    return await this.getById(_id)
  }

  public async updateNote(param): Promise<Document> {
    const db_cvote = this.getDBModel('CVote')
    const { _id, notes } = param

    const cur = await db_cvote.findOne({ _id })
    if (!cur) {
      throw 'invalid proposal id'
    }
    if (!this.canManageProposal()) {
      throw 'cvoteservice.updateNote - not council'
    }
    if (this.currentUser.role !== constant.USER_ROLE.SECRETARY) {
      throw 'only secretary could update notes'
    }

    const rs = await db_cvote.update({ _id }, {
      $set: {
        notes: notes || ''
      }
    })

    return await this.getById(_id)
  }

  private async eachJob() {
    const db_cvote = this.getDBModel('CVote')
    const list = await db_cvote.find({
      status: constant.CVOTE_STATUS.PROPOSED
    })
    const idsDeferred = []
    const idsActive = []
    const idsRejected = []

    _.each(list, (item) => {
      if (this.isExpired(item)) {
        if (this.isActive(item)) {
          idsActive.push(item._id)
        } else if (this.isRejected(item)) {
          idsRejected.push(item._id)
        } else {
          idsDeferred.push(item._id)
        }
      }
    })
    await db_cvote.update({
      _id: {
        $in: idsDeferred
      }
    }, {
      status: constant.CVOTE_STATUS.DEFERRED
    }, { multi: true })
    await db_cvote.update({
      _id: {
        $in: idsActive
      }
    }, {
      status: constant.CVOTE_STATUS.ACTIVE
    }, { multi: true })
    await db_cvote.update({
      _id: {
        $in: idsRejected
      }
    }, {
      status: constant.CVOTE_STATUS.REJECT
    }, { multi: true })

    this.notifyCouncilToVote()
  }

  public cronjob() {
    if (tm) {
      return false
    }
    tm = setInterval(() => {
      console.log('---------------- start cvote cronjob -------------')
      this.eachJob()
    }, 1000 * 60)
  }

  private canManageProposal() {
    const userRole = _.get(this.currentUser, 'role')
    return permissions.isCouncil(userRole) || permissions.isSecretary(userRole)
  }
}
