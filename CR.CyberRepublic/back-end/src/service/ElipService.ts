import Base from './Base'
import { Document } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../constant'
import { mail, logger, user as userUtil } from '../utility'
import * as moment from 'moment'

let tm = undefined

const BASE_FIELDS = [
  'title',
  'abstract',
  'specifications',
  'motivation',
  'rationale',
  'backwardCompatibility',
  'referenceImplementation',
  'copyright'
]
export default class extends Base {
  public async update(param: any): Promise<Document> {
    try {
      const db_elip = this.getDBModel('Elip')
      const { _id, status } = param
      const elipStatus = constant.ELIP_STATUS
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
          elipStatus.WAIT_FOR_REVIEW,
          elipStatus.FINAL_REVIEW,
          elipStatus.SUBMITTED_AS_PROPOSAL
        ].includes(elip.status)
      ) {
        throw `ElipService.update - can not update a ${status} elip`
      }
      if (
        status === elipStatus.FINAL_REVIEW &&
        elip.status !== elipStatus.DRAFT
      ) {
        throw `ElipService.update - can not change elip status to final review`
      }

      const fields = [...BASE_FIELDS, 'elipType']
      const doc: any = {}
      for (let i = 0; i < fields.length; i++) {
        const value = param[fields[i]]
        if (!value) {
          continue
        } else {
          if (fields[i] === 'elipType' && !constant.ELIP_TYPE[value]) {
            continue
          }
          doc[fields[i]] = value
        }
      }
      
      if (
        status === elipStatus.FINAL_REVIEW &&
        elip.status === elipStatus.DRAFT
      ) {
        doc.status = elipStatus.FINAL_REVIEW
        const rs = await db_elip.update({ _id }, doc)
        const author = userUtil.formatUsername(this.currentUser)
        this.notifySecretaries(this.finalReviewMailTemplate(author, elip.vid, elip._id))
        return rs
      }

      if (_.values(doc).length) {
        if (
          status === elipStatus.WAIT_FOR_REVIEW &&
          [elipStatus.REJECTED, elipStatus.PERSONAL_DRAFT].includes(
            elip.status
          )
        ) {
          doc.status = elipStatus.WAIT_FOR_REVIEW
          const rs = await db_elip.update({ _id }, doc)
          const title = doc.title ? doc.title : elip.title
          if (elip.status === elipStatus.REJECTED) {
            this.notifySecretaries(this.updateMailTemplate(title, elip._id))
          }
          if (elip.status === elipStatus.PERSONAL_DRAFT) {
            this.notifySecretaries(this.createMailTemplate(title, elip._id))
          }
          return rs
        }

        if (
          status === elip.status &&
          [elipStatus.PERSONAL_DRAFT, elipStatus.DRAFT].includes(elip.status)
        ) {
          const rs = await db_elip.update({ _id }, doc)
          return rs
        }
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

      const { status } = param
      const fields = [...BASE_FIELDS, 'elipType']
      const doc: any = {}
      for (let i = 0; i < fields.length; i++) {
        const value = param[fields[i]]
        if (fields[i] === 'elipType' && !constant.ELIP_TYPE[value]) {
          doc.elipType = _.values(constant.ELIP_TYPE)[0]
        } else {
          doc[fields[i]] = value
        }
      }
      
      if (
        [
          constant.ELIP_STATUS.PERSONAL_DRAFT,
          constant.ELIP_STATUS.WAIT_FOR_REVIEW
        ].includes(status)
      ) {
        doc.status = status
      }
      
      doc.contentType = constant.CONTENT_TYPE.MARKDOWN
      doc.createdBy = this.currentUser._id

      if (status === constant.ELIP_STATUS.WAIT_FOR_REVIEW) {
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
      }
      
      const elip = await db_elip.save(doc)
      if (status === constant.ELIP_STATUS.WAIT_FOR_REVIEW) {
        this.notifySecretaries(this.createMailTemplate(elip.title, elip._id))
      }
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

  private updateMailTemplate(title: string, id: string) {
    const subject = 'An ELIP updated'
    const body = `
      <p>This is ELIP ${title} updated and to be reviewed:</p>
      <br />
      <p>Click this link to view more details: <a href="${process.env.SERVER_URL}/elips/${id}">${process.env.SERVER_URL}/elips/${id}</a></p>
      <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `
    return {subject, body}
  }

  private createMailTemplate(title: string, id: string) {
    const subject = 'New ELIP created'
    const body = `
      <p>This is a new ELIP ${title} added and to be reviewed:</p>
      <br />
      <p>Click this link to view more details: <a href="${process.env.SERVER_URL}/elips/${id}">${process.env.SERVER_URL}/elips/${id}</a></p>
      <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `
    return {subject, body}
  }

  private finalReviewMailTemplate(author: string, vid: number, id: string) {
    const subject = `[Final review needed] ELIP #${vid} submitted as proposal`
    const body = `
      <p>${author} has submitted ELIP #${vid} as proposal, please review it.</p>
      <br />
      <p>Click this link to view more details: <a href="${process.env.SERVER_URL}/elips/${id}">${process.env.SERVER_URL}/elips/${id}</a></p>
      <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
    `
    return {subject, body}
  }

  private async notifySecretaries(content: {subject: string, body: string}) {
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
      subject: content.subject,
      body: content.body,
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
    // access ELIP by reference number
    const isNumber = /^\d*$/.test(id)
    let query: any
    if (isNumber) {
      query = { vid: parseInt(id) }
    } else {
      query = { _id: id }
    }
    const rs = await db_elip
      .getDBInstance()
      .findOne(query)
      .populate(
        'voteResult.votedBy',
        constant.DB_SELECTED_FIELDS.USER.NAME_AVATAR
      )
      .populate('reference')
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)
    if (!rs) {
      return { elip: { success: true, empty: true } }
    }
    const db_elip_review = this.getDBModel('Elip_Review')
    const reviews = await db_elip_review
      .getDBInstance()
      .find({ elipId: rs._id })
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)

    const currentUserId = _.get(this.currentUser, '_id')
    const userRole = _.get(this.currentUser, 'role')

    const isVisible = rs.createdBy._id.equals(currentUserId) ||
      userRole === constant.USER_ROLE.SECRETARY ||
      userRole === constant.USER_ROLE.ADMIN ||
      [constant.ELIP_STATUS.DRAFT, constant.ELIP_STATUS.SUBMITTED_AS_PROPOSAL].includes(
        rs.status
      )

    if (_.isEmpty(rs.comments)) {
      return isVisible ? { elip: rs, reviews } : { elip: { success: true, empty: true } }
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

    return isVisible ? { elip: rs, reviews } : { elip: { success: true, empty: true } }
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

    const status = constant.ELIP_STATUS
    const privateStatus = [
      status.REJECTED,
      status.WAIT_FOR_REVIEW,
      status.PERSONAL_DRAFT,
      status.FINAL_REVIEW
    ]
    const publicStatus = [status.DRAFT, status.SUBMITTED_AS_PROPOSAL]

    if (!this.isLoggedIn()) {
      // guest
      if (param.filter && publicStatus.includes(param.filter)) {
        query.status = param.filter
      } else {
        query.status = { $in: publicStatus }
      }
    } else {
      // secretary and admin
      const role = constant.USER_ROLE
      if ([role.SECRETARY, role.ADMIN].includes(userRole)) {
        const specialStatus = _.values(status).filter(
          item => item === status.PERSONAL_DRAFT
        )
        if (!param.filter || param.filter === 'ALL') {
          query.$or = [
            {
              createdBy: currentUserId,
              status: { $in: status.PERSONAL_DRAFT }
            },
            { status: { $in: specialStatus} }
          ]
        }
        if (param.filter && param.filter === status.PERSONAL_DRAFT) {
          query.createdBy = currentUserId
          query.status = param.filter
        }
        if (param.filter && specialStatus.includes(param.filter)) {
          query.status = param.filter
        }
      } else {
        if (!param.filter || param.filter === 'ALL') {
          query.$or = [
            {
              createdBy: currentUserId,
              status: { $in: privateStatus }
            },
            { status: { $in: publicStatus } }
          ]
        }
        if (privateStatus.includes(param.filter)) {
          query.createdBy = currentUserId
          query.status = param.filter
        }
        if (publicStatus.includes(param.filter)) {
          query.status = param.filter
        }
      }
    }

    // createBy
    if (!_.isEmpty(param.author)) {
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
      const userIds = users.map((el: { _id: string }) => el._id)
      // prevent members to search private ELIPs
      if (!_.isEmpty(query.createdBy)) {
        const rs = userIds.filter((id: any) => id.equals(query.createdBy))
        if (!rs.length) {
          return []
        }
      } else {
        query.createdBy = { $in: userIds }
      }
    }
    
    // elipType
    if(param.type && _.has(constant.ELIP_TYPE, param.type)){
      query.elipType = param.type
    }

    // startDate <  endDate
    if(
      !_.isEmpty(param.startDate) &&
      !_.isEmpty(param.endDate) &&
      moment(param.endDate).isSameOrAfter(param.startDate)
    ) {
      query.createdAt = {
        $gte: moment(param.startDate),
        $lte: moment(param.endDate).add(1, 'd')
      }
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

    const elip = await db_elip
      .getDBInstance()
      .findOne({ _id: elipId })
      .populate(
        'voteResult.votedBy',
        constant.DB_SELECTED_FIELDS.USER.NAME_AVATAR
      )
      .populate('createdBy')
    
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

    let cvoteType = constant.CVOTE_TYPE.STANDARD_TRACK
    if( elip.elipType === constant.ELIP_TYPE.PROCESS ) {
      cvoteType = constant.CVOTE_TYPE.PROCESS
    }else if( elip.elipType === constant.ELIP_TYPE.INFORMATIONAL ) {
      cvoteType = constant.CVOTE_TYPE.INFORMATIONAL
    }

    const doc: any = {
      vid,
      type: cvoteType,
      status: constant.CVOTE_STATUS.PROPOSED,
      published: true,
      contentType: constant.CONTENT_TYPE.MARKDOWN,
      proposedBy: userUtil.formatUsername(creator),
      proposer: elip.createdBy,
      createdBy: this.currentUser._id,
      referenceElip:  elipId
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
          reference: res._id,
          status: constant.ELIP_STATUS.SUBMITTED_AS_PROPOSAL
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
