import Base from './Base'
import { Document } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../constant'
import { mail, logger, user as userUtil } from '../utility'
import * as moment from 'moment'

let tm = undefined

const BASE_FIELDS = ['title', 'abstract', 'specifications', 'motivation', 'rationale', 'backwardCompatibility', 'referenceImplementation', 'copyright'];
export default class extends Base {
  public async update(param: any): Promise<Document> {
    try {
      const db_elip = this.getDBModel('Elip')
      const { _id, status } = param
      const elip = await db_elip
        .getDBInstance()
        .findOne({ _id })
        .populate(
          'voteResult.votedBy',
          constant.DB_SELECTED_FIELDS.USER.NAME_AVATAR
        )
        .populate('createdBy')
      if (!elip) {
        throw 'ElipService.update - invalid elip id'
      }
      if (!elip.createdBy._id.equals(this.currentUser._id)) {
        throw 'ElipService.update - current user is not the author of elip'
      }
      if (
        [
          constant.ELIP_STATUS.WAIT_FOR_REVIEW,
          constant.ELIP_STATUS.SUBMITTED
        ].includes(elip.status)
      ) {
        throw `ElipService.update - can not update a ${status} elip`
      }
      if (
        status === constant.ELIP_STATUS.SUBMITTED &&
        elip.status !== constant.ELIP_STATUS.DRAFT
      ) {
        throw `ElipService.update - can not change elip status to submitted`
      }

      if (
        status === constant.ELIP_STATUS.SUBMITTED &&
        elip.status === constant.ELIP_STATUS.DRAFT
      ) {
        const rs = await db_elip.update(
          { _id },
          { status: constant.ELIP_STATUS.SUBMITTED }
        )
        this.notifySecretaries(elip, true)
        return rs
      }

      const { title,
              elipType,
              abstract,
              specifications,
              motivation,
              rationale,
              backwardCompatibility,
              referenceImplementation,
              copyright
            } = param
      const doc: any = {}
      if (title) {
        doc.title = title
      }
      if (elipType && constant.ELIP_TYPE[elipType]) {
        doc.elipType = elipType
      }
      if (abstract) {
        doc.abstract = abstract
      }
      if (specifications) {
        doc.specifications = specifications
      }
      if (motivation) {
        doc.motivation = motivation
      }
      if (rationale) {
        doc.rationale = rationale
      }
      if (backwardCompatibility) {
        doc.backwardCompatibility = backwardCompatibility
      }
      if (referenceImplementation) {
        doc.referenceImplementation = referenceImplementation
      }
      if (copyright) {
        doc.copyright = copyright
      }
      if (doc.title || doc.abstract
          || doc.elipType  || doc.specifications || doc.motivation
          || doc.rationale || doc.backwardCompatibility 
          || referenceImplementation || doc.copyright) {
        doc.status = constant.ELIP_STATUS.WAIT_FOR_REVIEW
        const rs = await db_elip.update({ _id }, doc)
        this.notifySecretaries(elip, true)
        return rs
      }
    } catch (error) {
      logger.error(error)
      return
    }
  }

  public async create(param: any): Promise<Document> {
    try {
      const db_elip = this.getDBModel('Elip')
      const db_user = this.getDBModel('User')
      let { title,
              elipType,
              abstract,
              specifications,
              motivation,
              rationale,
              backwardCompatibility,
              referenceImplementation,
              copyright
          } = param
      if(!constant.ELIP_TYPE[elipType]){
        elipType = _.values(constant.ELIP_TYPE)[0]
      }
      const vid = await this.getNewVid()
      const doc: any = {
        title,
        vid,
        elipType,
        abstract,
        specifications,
        motivation,
        rationale,
        backwardCompatibility,
        referenceImplementation,
        copyright,
        status: constant.ELIP_STATUS.WAIT_FOR_REVIEW,
        contentType: constant.CONTENT_TYPE.MARKDOWN,
        createdBy: this.currentUser._id
      }
      const councilMembers = await db_user.find({
        role: constant.USER_ROLE.COUNCIL
      })
      const voteResult = []
      _.map(councilMembers, user => {
        voteResult.push({
          votedBy: user._id,
          value: constant.ELIP_VOTE_RESULT.UNDECIDED
        })
      })
      doc.voteResult = voteResult
      doc.voteHistory = voteResult

      const elip = await db_elip.save(doc)
      this.notifySecretaries(elip)
      return elip
    } catch (error) {
      logger.error(error)
      return
    }
  }

  public async vote(param): Promise<Document> {
    const db_elip = this.getDBModel('Elip')
    const { _id, value, reason } = param
    const cur = await db_elip.findOne({ _id })
    const votedBy = _.get(this.currentUser, '_id')
    if (!cur) {
      throw 'invalid proposal id'
    }

    await db_elip.update(
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

  public async getNewVid() {
    const db_elip = this.getDBModel('Elip')
    const n = await db_elip.count({})
    return n + 1
  }

  private async notifySecretaries(elip: any, update?: boolean) {
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
    const subject = update ? 'An ELIP updated' : 'New ELIP created'
    const p = `This is ELIP ${elip.title} updated and to be reviewed:`
    const p1 = `This is a new ELIP ${elip.title} added and to be reviewed:`
    const body = `
      <p>${update ? p : p1}</p>
      <br />
      <p>Click this link to view more details: <a href="${
        process.env.SERVER_URL
      }/elips/${elip._id}">${process.env.SERVER_URL}/elips/${elip._id}</a></p>
      <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `

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

    mail.send(mailObj)
  }

  private async notifyCouncil(elip: any) {
    const db_user = this.getDBModel('User')
    const currentUserId = _.get(this.currentUser, '_id')
    const councilMembers = await db_user.find({
      role: constant.USER_ROLE.COUNCIL
    })
    const toUsers = _.filter(
      councilMembers,
      user => !user._id.equals(currentUserId)
    )
    const toMails = _.map(toUsers, 'email')

    const subject = `New ELIP: ${elip.title}`
    const body = `
      <p>There is a new ELIP added:</p>
      <br />
      <p>${elip.title}</p>
      <br />
      <p>Click this link to view more details: <a href="${
        process.env.SERVER_URL
      }/elips/${elip._id}">${process.env.SERVER_URL}/elips/${
      elip._id
    }</a></p>
      <br /> <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
    `

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
      // toName: ownerToName,
      subject,
      body,
      recVariables
    }

    mail.send(mailObj)
  }

  public async getById(id: string): Promise<any> {
    const db_elip = this.getDBModel('Elip')
    const rs = await db_elip
      .getDBInstance()
      .findById({ _id: id })
      .populate(
        'voteResult.votedBy',
        constant.DB_SELECTED_FIELDS.USER.NAME_AVATAR
      )
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
    if (!rs) {
      throw 'ElipService.getById - invalid elip id'
    }
    const db_elip_review = this.getDBModel('Elip_Review')
    const reviews = await db_elip_review
      .getDBInstance()
      .find({ elipId: id })
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)

    const currentUserId = _.get(this.currentUser, '_id')
    const userRole = _.get(this.currentUser, 'role')

    const isVisible = rs.createdBy._id.equals(currentUserId) ||
      userRole === constant.USER_ROLE.SECRETARY ||
      [constant.ELIP_STATUS.DRAFT, constant.ELIP_STATUS.SUBMITTED].includes(
        rs.status
      )

    if (_.isEmpty(rs.comments)) {
      return isVisible ? { elip: rs, reviews } : {}
    }

    for (const comment of rs.comments) {
      let promises = []
      for (const thread of comment) {
        promises.push(
          db_elip.getDBInstance().populate(thread, {
            path: 'createdBy',
            select: `${constant.DB_SELECTED_FIELDS.USER.NAME} profile.avatar`
          })
        )
      }
      await Promise.all(promises)
    }

    return isVisible ? { elip: rs, reviews } : {}
  }

  public async remove(_id : string): Promise<any> {
    const db_elip = this.getDBModel('Elip')
    let rs = await db_elip
      .getDBInstance()
      .findById({ _id })
      .populate(
        'voteResult.votedBy',
        constant.DB_SELECTED_FIELDS.USER.NAME_AVATAR
      )
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
    if (!rs) {
      throw 'ElipService.remove - invalid elip id'
    }

    const currentUserId = _.get(this.currentUser, '_id')
    const userRole = _.get(this.currentUser, 'role')
    if(userRole !== constant.USER_ROLE.ADMIN) {
      throw 'ElipService.remove - invalid user role'
    }
    
    rs = await db_elip.remove({ _id })

    return rs
  }

  public async list(param: any): Promise<any> {
    const db_elip = this.getDBModel('Elip')
    const currentUserId = _.get(this.currentUser, '_id')
    const userRole = _.get(this.currentUser, 'role')
    const query: any = {}

    if (!this.isLoggedIn()) {
      query.status = {
        $in: [constant.ELIP_STATUS.DRAFT, constant.ELIP_STATUS.SUBMITTED]
      }
      param.filter = null
    }

    if (param.filter === constant.ELIP_FILTER.DRAFT) {
      query.status = constant.ELIP_STATUS.DRAFT
    }

    if (param.filter === constant.ELIP_FILTER.SUBMITTED_BY_ME) {
      query.createdBy = currentUserId
      query.status = constant.ELIP_STATUS.SUBMITTED
    }

    if (param.filter === constant.ELIP_FILTER.WAIT_FOR_REVIEW) {
      query.status = constant.ELIP_STATUS.WAIT_FOR_REVIEW
    }

    if (
      this.isLoggedIn() &&
      userRole !== constant.USER_ROLE.SECRETARY &&
      param.filter === constant.ELIP_FILTER.ALL
    ) {
      query.$or = [
        {
          createdBy: currentUserId,
          status: {
            $in: [
              constant.ELIP_STATUS.REJECTED,
              constant.ELIP_STATUS.WAIT_FOR_REVIEW
            ]
          }
        },
        {
          status: {
            $in: [constant.ELIP_STATUS.DRAFT, constant.ELIP_STATUS.SUBMITTED]
          }
        }
      ]
    }

    if (param.$or && query.$or) {
      query.$and = [{ $or: query.$or }, { $or: param.$or }]
    }

    if (param.$or && !query.$or) {
      query.$or = param.$or
    }

    const fields = 'vid title createdBy createdAt status'
    const list = await db_elip
      .getDBInstance()
      .find(query, fields)
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
      .sort({ vid: -1 })
      .limit(100)

    return list
  }

  private async notifyCouncilToVote() {
    // find elip before 1 day expiration without vote yet for each council member
    const db_elip = this.getDBModel('Elip')
    const nearExpiredTime =
      Date.now() - (constant.ELIP_EXPIRATION - constant.ONE_DAY)
    const unvotedElips = await db_elip
      .getDBInstance()
      .find({
        proposedAt: {
          $lt: nearExpiredTime,
          $gt: Date.now() - constant.ELIP_EXPIRATION
        },
        notified: { $ne: true },
        status: constant.ELIP_STATUS.PROPOSED
      })
      .populate(
        'voteResult.votedBy',
        constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL
      )

    _.each(unvotedElips, elip => {
      _.each(elip.voteResult, result => {
        if (result.value === constant.ELIP_VOTE_RESULT.UNDECIDED) {
          // send email to council member to notify to vote
          const { title, _id } = elip
          const subject = `Proposal Vote Reminder: ${title}`
          const body = `
            <p>You only got 24 hours to vote this proposal:</p>
            <br />
            <p>${title}</p>
            <br />
            <p>Click this link to vote: <a href="${
              process.env.SERVER_URL
            }/elips/${_id}">${
            process.env.SERVER_URL
          }/elips/${_id}</a></p>
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
          db_elip.update({ _id: elip._id }, { $set: { notified: true } })
        }
      })
    })
  }

  public async getNewCVoteVid() {
    const db_cvote = this.getDBModel('CVote')
    const n = await db_cvote.count({})
    return n + 1
  }
  public async propose(elipId: string): Promise<Document> {
    const currentUserId = _.get(this.currentUser, '_id')
    const userRole = _.get(this.currentUser, 'role')
    const db_elip = this.getDBModel('Elip')
    const db_cvote = this.getDBModel('CVote')
    const db_user = this.getDBModel('User')

    const elip = elipId && (await db_elip.findById(elipId))
    if (!elip) {
      throw 'ElipService.propose - cannot find elip'
    }
    if (!elip.createdBy._id.equals(this.currentUser._id)) {
      throw 'ElipService.propose - current user is not the author of elip'
    }
    if ( elip.status !== constant.ELIP_STATUS.DRAFT ) {
      throw 'ElipService.propose - elip status not equal DRAFT '
    }
    
    const creator = await db_user.findById(elip.createdBy);
    const vid = await this.getNewCVoteVid()

    const doc: any = {
      vid,
      type: elip.elipType,
      status: constant.CVOTE_STATUS.PROPOSED,
      published: true,
      contentType: constant.CONTENT_TYPE.MARKDOWN,
      proposedBy: userUtil.formatUsername(creator),
      proposer: elip.createdBy,
      createdBy: this.currentUser._id,
      reference: elipId
    }

    Object.assign(doc, _.pick(elip, BASE_FIELDS));

    const councilMembers = await db_user.find({
      role: constant.USER_ROLE.COUNCIL
    })
    const voteResult = []
    doc.proposedAt = Date.now()
    _.each(councilMembers, user =>
      voteResult.push({
        votedBy: user._id,
        value: constant.CVOTE_RESULT.UNDECIDED
      })
    )
    doc.voteResult = voteResult
    doc.voteHistory = voteResult

    try {
      const res = await db_cvote.save(doc)
      await db_elip.update(
        { _id: elipId },
        {
          $addToSet: { reference: res._id },
          $set: {
            tags: [],
            status: constant.ELIP_STATUS.SUBMITTED
          }
        }
      )
      this.notifyCouncilAfterPropose(res)
      return res
    } catch (error) {
      logger.error(error)
      return
    }
  }

  private async notifyCouncilAfterPropose(cvote: any) {
    const db_user = this.getDBModel('User')
    const currentUserId = _.get(this.currentUser, '_id')
    const councilMembers = await db_user.find({
      role: constant.USER_ROLE.COUNCIL
    })
    const toUsers = _.filter(
      councilMembers,
      user => !user._id.equals(currentUserId)
    )
    const toMails = _.map(toUsers, 'email')

    const subject = `New Proposal: ${cvote.title}`
    const body = `
      <p>There is a new proposal added:</p>
      <br />
      <p>${cvote.title}</p>
      <br />
      <p>Click this link to view more details: <a href="${
        process.env.SERVER_URL
      }/proposals/${cvote._id}">${process.env.SERVER_URL}/proposals/${
      cvote._id
    }</a></p>
      <br /> <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
    `

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
      // toName: ownerToName,
      subject,
      body,
      recVariables
    }

    mail.send(mailObj)
  }

  public isExpired(data: any, extraTime = 0): Boolean {
    const ct = moment(data.proposedAt || data.createdAt).valueOf()
    if (Date.now() - ct - extraTime > constant.ELIP_EXPIRATION) {
      return true
    }
    return false
  }

  // proposal active/passed
  public isActive(data): Boolean {
    const supportNum =
      _.countBy(data.voteResult, 'value')[constant.ELIP_VOTE_RESULT.SUPPORT] || 0
    return supportNum > data.voteResult.length * 0.5
  }

  // proposal rejected
  public isRejected(data): Boolean {
    const rejectNum =
      _.countBy(data.voteResult, 'value')[constant.ELIP_VOTE_RESULT.REJECT] || 0
    return rejectNum > data.voteResult.length * 0.5
  }

  private async eachJob() {
    const db_elip = this.getDBModel('Elip')
    const list = await db_elip.find({
      // wait requirement 
      status: constant.ELIP_STATUS.PROPOSED
    })
    const idsDeferred = []
    const idsActive = []
    const idsRejected = []

    _.each(list, item => {
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
    await db_elip.update(
      {
        _id: {
          $in: idsDeferred
        }
      },
      {
        status: constant.ELIP_STATUS.DEFERRED
      },
      { multi: true }
    )
    await db_elip.update(
      {
        _id: {
          $in: idsActive
        }
      },
      {
        status: constant.ELIP_STATUS.ACTIVE
      },
      { multi: true }
    )
    await db_elip.update(
      {
        _id: {
          $in: idsRejected
        }
      },
      {
        status: constant.ELIP_STATUS.REJECT
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
      console.log('---------------- start elip cronjob -------------')
      this.eachJob()
    }, 1000 * 60)
  }
}
