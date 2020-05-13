import Base from './Base'
import { Document } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../constant'
import { permissions, getDidPublicKey } from '../utility'
import * as moment from 'moment'
import * as jwt from 'jsonwebtoken'
import {
  mail,
  utilCrypto,
  user as userUtil,
  timestamp,
  logger
} from '../utility'
const util = require('util')
const request = require('request')

let tm = undefined

const BASE_FIELDS = [
  'title',
  'abstract',
  'goal',
  'motivation',
  'relevance',
  'budget',
  'budgetAmount',
  'elaAddress',
  'plan',
  'payment'
]

const WALLET_STATUS_TO_CVOTE_STATUS = {
  ALL: [
    constant.CVOTE_STATUS.PROPOSED,
    constant.CVOTE_STATUS.NOTIFICATION,
    constant.CVOTE_STATUS.ACTIVE,
    constant.CVOTE_STATUS.FINAL,
    constant.CVOTE_STATUS.REJECT,
    constant.CVOTE_STATUS.DEFERRED
  ],
  VOTING: [constant.CVOTE_STATUS.PROPOSED],
  NOTIFICATION: [constant.CVOTE_STATUS.NOTIFICATION],
  ACTIVE: [constant.CVOTE_STATUS.ACTIVE],
  FINAL: [constant.CVOTE_STATUS.FINAL],
  REJECTED: [constant.CVOTE_STATUS.REJECT, constant.CVOTE_STATUS.DEFERRED]
}

const CVOTE_STATUS_TO_WALLET_STATUS = {
  [constant.CVOTE_STATUS.PROPOSED]: 'VOTING',
  [constant.CVOTE_STATUS.NOTIFICATION]: 'NOTIFICATION',
  [constant.CVOTE_STATUS.ACTIVE]: 'ACTIVE',
  [constant.CVOTE_STATUS.FINAL]: 'FINAL',
  [constant.CVOTE_STATUS.REJECT]: 'REJECTED',
  [constant.CVOTE_STATUS.DEFERRED]: 'REJECTED'
}

export default class extends Base {
  // create a DRAFT propoal with minimal info
  public async createDraft(param: any): Promise<Document> {
    const db_suggestion = this.getDBModel('Suggestion')
    const db_cvote = this.getDBModel('CVote')
    const { title, proposedBy, proposer, suggestionId, payment } = param

    const vid = await this.getNewVid()
    const userRole = _.get(this.currentUser, 'role')
    if (!this.canCreateProposal()) {
      throw 'cvoteservice.create - no permission'
    }

    const doc: any = {
      title,
      vid,
      payment,
      status: constant.CVOTE_STATUS.DRAFT,
      published: false,
      contentType: constant.CONTENT_TYPE.MARKDOWN,
      proposedBy,
      proposer: proposer ? proposer : this.currentUser._id,
      createdBy: this.currentUser._id
    }
    const suggestion =
      suggestionId && (await db_suggestion.findById(suggestionId))
    if (!_.isEmpty(suggestion)) {
      doc.reference = suggestionId
    }

    Object.assign(doc, _.pick(suggestion, BASE_FIELDS))

    try {
      return await db_cvote.save(doc)
    } catch (error) {
      logger.error(error)
      return
    }
  }

  public async proposeSuggestion(param: any): Promise<Document> {
    const db_suggestion = this.getDBModel('Suggestion')
    const db_cvote = this.getDBModel('CVote')
    const db_user = this.getDBModel('User')
    const { suggestionId } = param

    const suggestion =
      suggestionId && (await db_suggestion.findById(suggestionId))
    if (!suggestion) {
      throw 'cannot find suggestion'
    }

    const creator = await db_user.findById(suggestion.createdBy)
    const vid = await this.getNewVid()

    const doc: any = {
      vid,
      type: suggestion.type,
      status: constant.CVOTE_STATUS.PROPOSED,
      published: true,
      contentType: constant.CONTENT_TYPE.MARKDOWN,
      proposedBy: userUtil.formatUsername(creator),
      proposer: suggestion.createdBy,
      createdBy: this.currentUser._id,
      reference: suggestionId
    }

    Object.assign(doc, _.pick(suggestion, BASE_FIELDS))

    const councilMembers = await db_user.find({
      role: constant.USER_ROLE.COUNCIL
    })
    const voteResult = []
    doc.proposedAt = Date.now()
    _.each(councilMembers, (user) =>
      voteResult.push({
        votedBy: user._id,
        value: constant.CVOTE_RESULT.UNDECIDED
      })
    )
    doc.voteResult = voteResult
    doc.voteHistory = voteResult

    try {
      const res = await db_cvote.save(doc)
      await db_suggestion.update(
        { _id: suggestionId },
        {
          $addToSet: { reference: res._id },
          $set: { tags: [] }
        }
      )
      this.notifySubscribers(res)
      this.notifyCouncil(res)
      return res
    } catch (error) {
      logger.error(error)
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
      _id,
      title,
      type,
      abstract,
      goal,
      motivation,
      relevance,
      budget,
      plan,
      payment
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
      contentType: constant.CONTENT_TYPE.MARKDOWN
    }

    if (title) doc.title = title
    if (type) doc.type = type
    if (abstract) doc.abstract = abstract
    if (goal) doc.goal = goal
    if (motivation) doc.motivation = motivation
    if (relevance) doc.relevance = relevance
    if (budget) {
      doc.budget = budget
    }
    if (plan) doc.plan = plan
    if (payment) doc.payment = payment

    try {
      await db_cvote.update({ _id }, doc)
      const res = await this.getById(_id)
      return res
    } catch (error) {
      logger.error(error)
      return
    }
  }

  // delete draft proposal by proposal id
  public async deleteDraft(param: any): Promise<any> {
    try {
      const db_cvote = this.getDBModel('CVote')
      const { _id } = param
      const doc = await db_cvote.findOne({ _id })
      if (!doc) {
        throw 'cvoteservice.deleteDraft - invalid proposal id'
      }
      if (doc.status !== constant.CVOTE_STATUS.DRAFT) {
        throw 'cvoteservice.deleteDraft - not draft proposal'
      }
      return await db_cvote.remove({ _id })
    } catch (error) {
      logger.error(error)
    }
  }

  public async create(param): Promise<Document> {
    const db_cvote = this.getDBModel('CVote')
    const db_user = this.getDBModel('User')
    const db_suggestion = this.getDBModel('Suggestion')
    const {
      title,
      published,
      proposedBy,
      proposer,
      suggestionId,
      abstract,
      goal,
      motivation,
      relevance,
      budget,
      plan,
      payment
    } = param

    const vid = await this.getNewVid()
    const status = published
      ? constant.CVOTE_STATUS.PROPOSED
      : constant.CVOTE_STATUS.DRAFT

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
      payment,
      proposer,
      createdBy: this.currentUser._id
    }

    const suggestion =
      suggestionId && (await db_suggestion.findById(suggestionId))
    if (!_.isEmpty(suggestion)) {
      doc.reference = suggestionId
    }

    const councilMembers = await db_user.find({
      role: constant.USER_ROLE.COUNCIL
    })
    const voteResult = []
    if (published) {
      doc.proposedAt = Date.now()
      _.each(councilMembers, (user) =>
        voteResult.push({
          votedBy: user._id,
          value: constant.CVOTE_RESULT.UNDECIDED
        })
      )
      doc.voteResult = voteResult
      doc.voteHistory = voteResult
    }

    try {
      const res = await db_cvote.save(doc)
      // add reference with suggestion
      if (!_.isEmpty(suggestion)) {
        await db_suggestion.update(
          { _id: suggestionId },
          { $addToSet: { reference: res._id } }
        )
        // notify creator and subscribers
        if (published) this.notifySubscribers(res)
      }

      // notify council member to vote
      if (published) this.notifyCouncil(res)

      return res
    } catch (error) {
      logger.error(error)
      return
    }
  }

  private async notifySubscribers(cvote: any) {
    const db_suggestion = this.getDBModel('Suggestion')
    const suggestionId = _.get(cvote, 'reference')
    if (!suggestionId) return
    const suggestion = await db_suggestion
      .getDBInstance()
      .findById(suggestionId)
      .populate('subscribers.user', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)

    const councilMember = await this.getDBModel('User').findById(
      cvote.createdBy
    )

    // get users: creator and subscribers
    const toUsers = _.map(suggestion.subscribers, 'user') || []
    toUsers.push(suggestion.createdBy)
    const toMails = _.map(toUsers, 'email')

    // compose email object
    const subject = `The suggestion is referred in Proposal #${cvote.vid}`
    const body = `
      <p>Council member ${userUtil.formatUsername(
        councilMember
      )} has refer to your suggestion ${suggestion.title} in a proposal #${
      cvote.vid
    }.</p>
      <br />
      <p>Click this link to view more details:</p>
      <p><a href="${process.env.SERVER_URL}/proposals/${cvote._id}">${
      process.env.SERVER_URL
    }/proposals/${cvote._id}</a></p>
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

    // send email
    mail.send(mailObj)
  }

  private async notifyCouncil(cvote: any) {
    const db_user = this.getDBModel('User')
    const currentUserId = _.get(this.currentUser, '_id')
    const councilMembers = await db_user.find({
      role: constant.USER_ROLE.COUNCIL
    })
    const toUsers = _.filter(
      councilMembers,
      (user) => !user._id.equals(currentUserId)
    )
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
  }

  private async notifyCouncilToVote() {
    // find cvote before 1 day expiration without vote yet for each council member
    const db_cvote = this.getDBModel('CVote')
    const nearExpiredTime =
      Date.now() - (constant.CVOTE_EXPIRATION - constant.ONE_DAY)
    const unvotedCVotes = await db_cvote
      .getDBInstance()
      .find({
        proposedAt: {
          $lt: nearExpiredTime,
          $gt: Date.now() - constant.CVOTE_EXPIRATION
        },
        notified: { $ne: true },
        status: constant.CVOTE_STATUS.PROPOSED
      })
      .populate(
        'voteResult.votedBy',
        constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL
      )

    _.each(unvotedCVotes, (cvote) => {
      _.each(cvote.voteResult, (result) => {
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
            body
          }
          mail.send(mailObj)

          // update notified to true
          db_cvote.update({ _id: cvote._id }, { $set: { notified: true } })
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
  public async list(param): Promise<Object> {
    const db_cvote = this.getDBModel('CVote')
    const currentUserId = _.get(this.currentUser, '_id')
    const userRole = _.get(this.currentUser, 'role')
    const query: any = {}

    if (!param.published) {
      if (!this.isLoggedIn() || !this.canManageProposal()) {
        throw 'cvoteservice.list - unpublished proposals only visible to council/secretary'
      } else if (
        param.voteResult === constant.CVOTE_RESULT.UNDECIDED &&
        permissions.isCouncil(userRole)
      ) {
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
    // createBy
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
    // cvoteType
    if (
      param.type &&
      _.indexOf(_.values(constant.CVOTE_TYPE), param.type) >= 0
    ) {
      query.type = param.type
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
    // Ends in times - 7day = startDate <  endDate
    if (
      param.endsInStartDate &&
      param.endsInStartDate.length &&
      param.endsInEndDate &&
      param.endsInEndDate.length
    ) {
      let endDate = new Date(
        new Date(param.endsInEndDate).getTime() - 7 * 24 * 3600 * 1000
      )
      endDate.setDate(endDate.getDate() + 1)
      query.createdAt = {
        $gte: new Date(
          new Date(param.endsInStartDate).getTime() - 7 * 24 * 3600 * 1000
        ),
        $lte: endDate
      }
      query.status = {
        $in: [
          constant.CVOTE_STATUS.PROPOSED,
          constant.CVOTE_STATUS.ACTIVE,
          constant.CVOTE_STATUS.REJECT,
          constant.CVOTE_STATUS.FINAL,
          constant.CVOTE_STATUS.DEFERRED,
          constant.CVOTE_STATUS.INCOMPLETED
        ]
      }
    }
    // status
    if (param.status && constant.CVOTE_STATUS[param.status]) {
      query.status = param.status
    }
    // budget
    if (param.budgetLow || param.budgetHigh) {
      query.budgetAmount = {}
      if (param.budgetLow && param.budgetLow.length) {
        query.budgetAmount['$gte'] = parseInt(param.budgetLow)
      }
      if (param.budgetHigh && param.budgetHigh.length) {
        query.budgetAmount['$lte'] = parseInt(param.budgetHigh)
      }
    }
    // has tracking
    if (param.hasTracking) {
      const db_cvote_tracking = this.getDBModel('CVote_Tracking')
      const hasTracking = await db_cvote_tracking.find(
        {
          status: constant.CVOTE_TRACKING_STATUS.REVIEWING
        },
        'proposalId'
      )
      let trackingProposals = []
      hasTracking.map(function (it) {
        trackingProposals.push(it.proposalId)
      })
      query._id = {
        $in: trackingProposals
      }
    }

    if (param.$or) query.$or = param.$or
    const fields = [
      'vid',
      'title',
      'type',
      'proposedBy',
      'status',
      'published',
      'proposedAt',
      'createdAt',
      'voteResult',
      'vote_map'
    ]

    // const list = await db_cvote.list(query, { vid: -1 }, 0, fields.join(' '))

    const cursor = db_cvote
      .getDBInstance()
      .find(query, fields.join(' '))
      .sort({ vid: -1 })

    if (param.results) {
      const results = parseInt(param.results, 10)
      const page = parseInt(param.page, 10)
      cursor.skip(results * (page - 1)).limit(results)
    }

    const rs = await Promise.all([
      cursor,
      db_cvote.getDBInstance().find(query).count()
    ])
    const list = rs[0]
    const total = rs[1]

    return { list, total }
  }

  /**
   *
   * @param param
   * @returns {Promise<"mongoose".Document>}
   */
  public async update(param): Promise<Document> {
    const db_user = this.getDBModel('User')
    const db_cvote = this.getDBModel('CVote')
    const {
      _id,
      published,
      notes,
      title,
      abstract,
      goal,
      motivation,
      relevance,
      budget,
      plan
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
      contentType: constant.CONTENT_TYPE.MARKDOWN
    }
    const willChangeToPublish =
      published === true && cur.status === constant.CVOTE_STATUS.DRAFT

    if (title) doc.title = title
    if (abstract) doc.abstract = abstract
    if (goal) doc.goal = goal
    if (motivation) doc.motivation = motivation
    if (relevance) doc.relevance = relevance
    if (budget) {
      doc.budget = budget
    }
    if (plan) doc.plan = plan

    if (willChangeToPublish) {
      doc.status = constant.CVOTE_STATUS.PROPOSED
      doc.published = published
      doc.proposedAt = Date.now()
      const councilMembers = await db_user.find({
        role: constant.USER_ROLE.COUNCIL
      })
      const voteResult = []
      _.each(councilMembers, (user) =>
        voteResult.push({
          votedBy: user._id,
          value: constant.CVOTE_RESULT.UNDECIDED
        })
      )
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
      logger.error(error)
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

    const rs = await db_cvote.update(
      { _id: id },
      {
        $set: {
          status: constant.CVOTE_STATUS.FINAL
        }
      }
    )

    return rs
  }

  public async unfinishById(id): Promise<any> {
    const db_cvote = this.getDBModel('CVote')
    const cur = await db_cvote.findOne({ _id: id })
    if (!cur) {
      throw 'invalid proposal id'
    }
    if (!this.canManageProposal()) {
      throw 'cvoteservice.unfinishById - not council'
    }
    if (
      _.includes(
        [constant.CVOTE_STATUS.FINAL, constant.CVOTE_STATUS.INCOMPLETED],
        cur.status
      )
    ) {
      throw 'proposal already completed.'
    }

    const rs = await db_cvote.update(
      { _id: id },
      {
        $set: {
          status: constant.CVOTE_STATUS.INCOMPLETED
        }
      }
    )

    return rs
  }

  public async getById(id): Promise<any> {
    const db_cvote = this.getDBModel('CVote')
    // access proposal by reference number
    const isNumber = /^\d*$/.test(id)
    let query: any
    if (isNumber) {
      query = { vid: parseInt(id) }
    } else {
      query = { _id: id }
    }
    const rs = await db_cvote
      .getDBInstance()
      .findOne(query)
      .populate(
        'voteResult.votedBy',
        constant.DB_SELECTED_FIELDS.USER.NAME_AVATAR
      )
      .populate('proposer', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
      .populate('reference', constant.DB_SELECTED_FIELDS.SUGGESTION.ID)
      .populate('referenceElip', 'vid')
    if (!rs) {
      return { success: true, empty: true }
    }
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
    const supportNum =
      _.countBy(data.voteResult, 'value')[constant.CVOTE_RESULT.SUPPORT] || 0
    return supportNum > data.voteResult.length * 0.5
  }

  // proposal rejected
  public isRejected(data): Boolean {
    const rejectNum =
      _.countBy(data.voteResult, 'value')[constant.CVOTE_RESULT.REJECT] || 0
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
          'voteResult.$.reason': reason || ''
        },
        $push: {
          voteHistory: {
            value,
            reason,
            votedBy
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

    const rs = await db_cvote.update(
      { _id },
      {
        $set: {
          notes: notes || ''
        }
      }
    )

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
    await db_cvote.update(
      {
        _id: {
          $in: idsDeferred
        }
      },
      {
        status: constant.CVOTE_STATUS.DEFERRED
      },
      { multi: true }
    )
    await db_cvote.update(
      {
        _id: {
          $in: idsActive
        }
      },
      {
        status: constant.CVOTE_STATUS.ACTIVE
      },
      { multi: true }
    )
    await db_cvote.update(
      {
        _id: {
          $in: idsRejected
        }
      },
      {
        status: constant.CVOTE_STATUS.REJECT
      },
      { multi: true }
    )

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
  private canCreateProposal() {
    const userRole = _.get(this.currentUser, 'role')
    return (
      !permissions.isCouncil(userRole) && !permissions.isSecretary(userRole)
    )
  }

  public async listcrcandidates(param) {
    const { pageNum, pageSize, state } = param

    let ret = null
    // url: 'http://54.223.244.60/api/dposnoderpc/check/listcrcandidates',
    const postPromise = util.promisify(request.post, { multiArgs: true })
    await postPromise({
      url:
        'https://unionsquare.elastos.org/api/dposnoderpc/check/listcrcandidates',
      form: { pageNum, pageSize, state },
      encoding: 'utf8'
    }).then((value) => (ret = value.body))

    return ret
  }

  // council vote onchain
  public async onchain(param) {
    try {
      const db_cvote = this.getDBModel('CVote')
      const userId = _.get(this.currentUser, '_id')
      const { id } = param
      const jwtClaims = {
        iss: process.env.APP_DID,
        callbackurl: `${process.env.API_URL}/api/CVote/callback`,
        website: {
          domain: process.env.SERVER_URL,
          logo: `${process.env.SERVER_URL}/assets/images/logo.svg`
        },
        command: '',
        data: {
          proposalHash: '',
          voteResult: [],
          opinionHash: '',
          did: ''
        }
      }
      const cur = await db_cvote.findOne({ _id: id })

      if (!this.canManageProposal()) {
        throw 'cvoteservice.unfinishById - not council'
      }
      if (!cur) {
        throw 'cvoteservice.update - invalid proposal id'
      }

      jwtClaims.data.proposalHash = cur.proposalHash
      jwtClaims.data.voteResult = cur.voteResult

      cur.voteResult.forEach(function (res) {
        if (res.votedBy.equals(userId)) {
          jwtClaims.data.opinionHash = utilCrypto.sha256(
            utilCrypto.sha256(res.reason)
          )
        }
      })

      const jwtToken = jwt.sign(jwtClaims, process.env.APP_PRIVATE_KEY, {
        expiresIn: '7d',
        algorithm: 'ES256'
      })
      const url = `elastos://credaccess/${jwtToken}`
      return { success: true, url }
    } catch (err) {
      logger.error(err)
      return { success: false }
    }
  }

  // council callback
  public async councilCallback(param): Promise<any> {
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
        claims.req.slice('elastos://credaccess/'.length)
      )
      if (!_.get(payload.data, 'proposalHash')) {
        return {
          code: 400,
          success: false,
          message: 'Problems parsing jwt token of CR website.'
        }
      }

      const db_cvote = this.getDBModel('CVote')
      const cur = await db_cvote.find({
        proposalHash: payload.data.proposalHash
      })
      if (!cur) {
        return {
          code: 400,
          success: false,
          message: 'There is no this proposal'
        }
      }
      const votedBy = _.get(this.currentUser, '_id')
      const proposalHash = payload.data.proposalHash

      const voteResult = _.find(cur[0].voteResult, function (o) {
        if (o.votedBy.equals(votedBy)) {
          return o
        }
      })
      const rs: any = await getDidPublicKey(claims.iss)
      if (!rs) {
        await db_cvote.update(
          {
            proposalHash: proposalHash,
            'voteResult.votedBy': votedBy
          },
          {
            $set: {
              'voteResult.$.signature': {
                message: `Can not get your did's public key. `
              }
            },
            $push: {
              voteHistory: {
                value: voteResult.value,
                reason: voteResult.reason,
                txid: voteResult.txid,
                votedBy: voteResult.votedBy,
                signature: voteResult.signature
              }
            }
          }
        )
        return {
          code: 400,
          success: false,
          message: `Can not get your did's public key`
        }
      }

      return jwt.verify(
        jwtToken,
        rs.publicKey,
        async (err: any, decoded: any) => {
          if (err) {
            await db_cvote.update(
              {
                proposalHash: proposalHash,
                'voteResult.votedBy': votedBy
              },
              {
                $set: {
                  'voteResult.$.signature': {
                    message: `Verify signatrue failed. `
                  }
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
              await db_cvote.update(
                {
                  proposalHash: proposalHash,
                  'voteResult.votedBy': votedBy
                },
                {
                  $set: {
                    'voteResult.$.txid': claims.data,
                    'voteResult.$.status': constant.CVOTE_CHAIN_STATUS.CHAINING,
                    'voteResult.$.signature': { data: decoded.data }
                  },
                  $push: {
                    voteHistory: {
                      value: voteResult.value,
                      reason: voteResult.reason,
                      txid: voteResult.txid,
                      votedBy: voteResult.votedBy,
                      signature: voteResult.signature
                    }
                  }
                }
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
    } catch (error) {
      logger.error(error)
      return {
        code: 500,
        success: false,
        message: 'Something went wrong'
      }
    }
  }

  public async checkSignature(param: any) {
    const { id } = param
    const db_cvote = this.getDBModel('CVote')
    const proposal = await db_cvote.find({ _id: id })
    if (proposal) {
      const signature = _.get(proposal, 'proposal.voteResult.signature')
      if (signature) {
        return { success: true, data: proposal }
      }
      const message = _.get(proposal, 'proposal.voteResult.message')
      if (message) {
        return { success: true, data: proposal }
      }
    } else {
      return {
        success: false
      }
    }
  }

  // according to txid polling vote status
  public async pollVoteChain() {
    const txid = ''
    const status = ''

    const db_cvote = this.getDBModel('CVote')

    try {
      await db_cvote.update(
        {
          'voteResult.txid': txid
        },
        {
          $set: {
            'voteResult.$.status': status
          }
        }
      )
    } catch (err) {
      logger.error(err)
    }
  }

  // member callback
  public async memberCallback(param): Promise<any> {
    return
  }

  /**
   * API to Wallet
   */
  public async allOrSearch(param): Promise<any> {
    const db_cvote = this.getDBModel('CVote')
    const query: any = {}

    if (
      !param.status ||
      !_.keys(WALLET_STATUS_TO_CVOTE_STATUS).includes(param.status)
    ) {
      return {
        code: 400,
        message: 'Invalid request parameters - status',
        // tslint:disable-next-line:no-null-keyword
        data: null
      }
    }

    // status
    query.status = WALLET_STATUS_TO_CVOTE_STATUS[param.status]

    // search
    if (param.$or) query.$or = param.$or

    const fields = [
      'vid',
      'title',
      'status',
      'createdAt',
      'proposedBy',
      'proposalHash'
    ]

    const cursor = db_cvote
      .getDBInstance()
      .find(query, fields.join(' '))
      .sort({ vid: -1 })

    if (
      param.page &&
      param.results &&
      parseInt(param.page) > 0 &&
      parseInt(param.results) > 0
    ) {
      const results = parseInt(param.results, 10)
      const page = parseInt(param.page, 10)

      cursor.skip(results * (page - 1)).limit(results)
    }

    const rs = await Promise.all([
      cursor,
      db_cvote.getDBInstance().find(query).count()
    ])

    // filter return data，add proposalHash to CVoteSchema
    const list = _.map(rs[0], function (o) {
      let temp = _.pick(o, fields)
      temp.status = CVOTE_STATUS_TO_WALLET_STATUS[temp.status]
      temp.createdAt = timestamp.second(temp.createdAt)
      return _.mapKeys(temp, function (value, key) {
        if (key == 'vid') {
          return 'id'
        } else {
          return key
        }
      })
    })

    const total = rs[1]
    return { list, total }
  }

  public async getProposalById(id): Promise<any> {
    const db_cvote = this.getDBModel('CVote')

    const fields = [
      'status',
      'abstract',
      'voteResult',
      'createdAt',
      'proposalHash'
    ]
    const proposal = await db_cvote
      .getDBInstance()
      .findOne({ vid: id }, fields.join(' '))
      .populate('voteResult.votedBy', constant.DB_SELECTED_FIELDS.USER.NAME)

    if (!proposal) {
      return {
        code: 400,
        message: 'Invalid request parameters',
        // tslint:disable-next-line:no-null-keyword
        data: null
      }
    }

    const address = `${process.env.SERVER_URL}/proposals/${proposal.id}`

    // duration
    const endTime = Math.round(proposal.createdAt.getTime() / 1000 + 604800)
    const nowTime = Math.round(new Date().getTime() / 1000)
    let duration = endTime - nowTime
    duration = duration > 0 ? duration : 0

    const proposalId = proposal._id

    const voteResultFields = ['value', 'reason', 'votedBy']
    // filter undecided vote result
    const filterVoteResult = _.filter(
      proposal._doc.voteResult,
      (o: any) => o.value !== constant.CVOTE_RESULT.UNDECIDED
    )
    // update vote result data
    const voteResult = _.map(filterVoteResult, (o: any) =>
      _.pick(
        {
          ...o._doc,
          votedBy: `${_.get(o, 'votedBy.profile.firstName')} ${_.get(
            o,
            'votedBy.profile.firstName'
          )}`
        },
        voteResultFields
      )
    )

    const tracking = await this.getTracking(proposalId)

    const summary = await this.getSummary(proposalId)

    return _.omit(
      {
        ...proposal._doc,
        createdAt: timestamp.second(proposal.createdAt),
        voteResult,
        address,
        duration,
        tracking,
        summary
      },
      ['_id']
    )
  }

  public async getTracking(id) {
    const db_tracking = this.getDBModel('CVote_Tracking')
    const proposalId = id
    const queryTrack: any = {
      proposalId
    }
    const fieldsTrack = [
      'comment',
      'content',
      'status',
      'createdAt',
      'updatedAt'
    ]
    const cursorTrack = db_tracking
      .getDBInstance()
      .find(queryTrack, fieldsTrack.join(' '))
      .populate('comment.createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
      .sort({ createdAt: 1 })
    // const totalCursorTrack = db_tracking.getDBInstance().find(queryTrack).count()

    const tracking = await cursorTrack
    // const totalTrack = await totalCursorTrack

    const list = _.map(tracking, function (o) {
      const comment = o._doc.comment
      const commentObj = {
        content: comment.content,
        createdBy: `${_.get(o, 'comment.createdBy.profile.firstName')} ${_.get(
          o,
          'comment.createdBy.profile.firstName'
        )}`
      }
      const obj = {
        ...o._doc,
        comment: commentObj,
        createdAt: timestamp.second(o.createdAt),
        updatedAt: timestamp.second(o.updatedAt)
      }
      return _.pick(obj, fieldsTrack)
    })
    return list
  }

  public async getSummary(id) {
    const db_summary = this.getDBModel('CVote_Summary')
    const proposalId = id
    const querySummary: any = {
      proposalId
    }
    const fieldsSummary = [
      'comment',
      'content',
      'status',
      'createdAt',
      'updatedAt'
    ]
    const cursorSummary = db_summary
      .getDBInstance()
      .find(querySummary, fieldsSummary.join(' '))
      .populate('comment.createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
      .sort({ createdAt: 1 })
    // const totalCursorSummary = db_summary.getDBInstance().find(querySummary).count()

    const summary = await cursorSummary
    // const totalSummary = await totalCursorSummary

    const list = _.map(summary, function (o) {
      const comment = o._doc.comment
      const commentObj = {
        content: comment.content,
        createdBy: `${_.get(o, 'comment.createdBy.profile.firstName')} ${_.get(
          o,
          'comment.createdBy.profile.firstName'
        )}`
      }
      const obj = {
        ...o._doc,
        comment: commentObj,
        createdAt: timestamp.second(o.createdAt),
        updatedAt: timestamp.second(o.updatedAt)
      }
      return _.pick(obj, fieldsSummary)
    })
    return list
  }
}
