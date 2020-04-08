import Base from './Base'
import { Document } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../constant'
import { mail, logger, user as userUtil } from '../utility'
import * as moment from 'moment'

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
  public async unsetAbandonVid() {
    const db_elip = this.getDBModel('Elip')
    
    // update old status vid
    const ret = await db_elip
      .getDBInstance()
      .update({
        status: {
          $in: [
            constant.ELIP_STATUS.PERSONAL_DRAFT,
            constant.ELIP_STATUS.WAIT_FOR_REVIEW,
            constant.ELIP_STATUS.REJECTED
          ]
        }
      }, {
        $unset: {vid: -1}
      })
  }
  
  public async update(param: any): Promise<Document> {
    try {
      const db_elip = this.getDBModel('Elip')
      const { _id, status } = param
      const elipStatus = constant.ELIP_STATUS
      const elip = await db_elip.getDBInstance().findOne({ _id })
      if (!elip) {
        throw 'ElipService.update - invalid elip id'
      }
      if (!elip.createdBy.equals(this.currentUser._id)) {
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

      if (status === elipStatus.CANCELLED && elip.status === elipStatus.DRAFT) {
        const rs = await db_elip.update(
          { _id },
          { status: elipStatus.CANCELLED }
        )
        return rs
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
        this.notifySecretaries(
          this.finalReviewMailTemplate(author, elip.vid, elip._id)
        )
        return rs
      }

      if (_.values(doc).length) {
        if (
          status === elipStatus.WAIT_FOR_REVIEW &&
          [elipStatus.REJECTED, elipStatus.PERSONAL_DRAFT].includes(elip.status)
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
      const { status } = param
      const isValidStatus = [
        constant.ELIP_STATUS.PERSONAL_DRAFT,
        constant.ELIP_STATUS.WAIT_FOR_REVIEW
      ].includes(status)

      if (!isValidStatus) {
        throw `ElipService.create - not valid status`
      }

      const doc: any = {}
      doc.status = status
      const fields = [...BASE_FIELDS, 'elipType']
      for (let i = 0; i < fields.length; i++) {
        const value = param[fields[i]]
        if (fields[i] === 'elipType' && !constant.ELIP_TYPE[value]) {
          doc.elipType = _.values(constant.ELIP_TYPE)[0]
        } else {
          doc[fields[i]] = value
        }
      }
      doc.createdBy = this.currentUser._id

      const elip = await this.getDBModel('Elip').save(doc)

      if (status === constant.ELIP_STATUS.WAIT_FOR_REVIEW) {
        this.notifySecretaries(this.createMailTemplate(elip.title, elip._id))
      }
      return elip
    } catch (error) {
      logger.error(error)
      return
    }
  }

  private updateMailTemplate(title: string, id: string) {
    const subject = 'An ELIP updated'
    const body = `
      <p>This is ELIP ${title} updated and to be reviewed:</p>
      <p>Click this link to view more details:</p>
      <p><a href="${process.env.SERVER_URL}/elips/${id}">${process.env.SERVER_URL}/elips/${id}</a></p>
      <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `
    return { subject, body }
  }

  private createMailTemplate(title: string, id: string) {
    const subject = 'New ELIP created'
    const body = `
      <p>This is a new ELIP ${title} added and to be reviewed:</p>
      <p>Click this link to view more details:</p>
      <p><a href="${process.env.SERVER_URL}/elips/${id}">${process.env.SERVER_URL}/elips/${id}</a></p>
      <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `
    return { subject, body }
  }

  private finalReviewMailTemplate(author: string, vid: number, id: string) {
    const subject = `[Final review needed] ELIP #${vid} submitted as proposal`
    const body = `
      <p>${author} has submitted ELIP #${vid} as proposal, please review it.</p>
      <p>Click this link to view more details:</p>
      <p><a href="${process.env.SERVER_URL}/elips/${id}">${process.env.SERVER_URL}/elips/${id}</a></p>
      <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `
    return { subject, body }
  }

  private async notifySecretaries(content: { subject: string; body: string }) {
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
      .populate('reference')
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

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

    const status = constant.ELIP_STATUS
    const publicStatus = [
      status.DRAFT,
      status.SUBMITTED_AS_PROPOSAL,
      status.CANCELLED
    ]
    const superRoles = [constant.USER_ROLE.SECRETARY, constant.USER_ROLE.ADMIN]
    const isVisible = rs.createdBy._id.equals(currentUserId) ||
      (superRoles.includes(userRole) && rs.status !== status.PERSONAL_DRAFT) ||
      publicStatus.includes(rs.status)

    if (!isVisible) {
      return { elip: { success: true, empty: true } }
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

    return { elip: rs, reviews }
  }

  public async remove(_id: string): Promise<any> {
    const db_elip = this.getDBModel('Elip')
    const elip = await db_elip.getDBInstance().findById({ _id })
    if (!elip) {
      throw 'ElipService.remove - invalid elip id'
    }
    const userRole = _.get(this.currentUser, 'role')
    if (userRole !== constant.USER_ROLE.ADMIN) {
      throw 'ElipService.remove - invalid user role'
    }
    if (elip.status !== constant.ELIP_STATUS.CANCELLED) {
      throw 'ElipService.remove - it is not a cancelled elip'
    }

    const rs = await db_elip.remove({ _id })
    return rs
  }

  public async list(param: any): Promise<any> {
    this.unsetAbandonVid()

    const db_elip = this.getDBModel('Elip')
    const currentUserId = _.get(this.currentUser, '_id')
    const userRole = _.get(this.currentUser, 'role')
    const query: any = {}

    const status = constant.ELIP_STATUS
    if (param.filter && !_.values(status).includes(param.filter)) {
      return []
    }

    const privateStatus = [
      status.REJECTED,
      status.WAIT_FOR_REVIEW,
      status.PERSONAL_DRAFT,
      status.FINAL_REVIEW
    ]
    const publicStatus = [
      status.DRAFT,
      status.SUBMITTED_AS_PROPOSAL,
      status.CANCELLED
    ]

    if (!this.isLoggedIn()) {
      // guest
      if (!param.filter) {
        query.status = { $in: publicStatus }
      } else {
        if (publicStatus.includes(param.filter)) {
          query.status = param.filter
        } else {
          return []
        }
      }
    } else {
      // secretary and admin
      const role = constant.USER_ROLE
      if ([role.SECRETARY, role.ADMIN].includes(userRole)) {
        if (!param.filter) {
          query.$or = [
            {
              createdBy: currentUserId,
              status: { $in: status.PERSONAL_DRAFT }
            },
            {
              status: {
                $in: _.values(status).filter(
                  item => item !== status.PERSONAL_DRAFT
                )
              }
            }
          ]
        } else {
          if (param.filter === status.PERSONAL_DRAFT) {
            query.createdBy = currentUserId
          }
          query.status = param.filter
        }
      } else {
        if (!param.filter) {
          query.$or = [
            {
              createdBy: currentUserId,
              status: { $in: privateStatus }
            },
            { status: { $in: publicStatus } }
          ]
        } else {
          if (privateStatus.includes(param.filter)) {
            query.createdBy = currentUserId
          }
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
    if (param.type && _.has(constant.ELIP_TYPE, param.type)) {
      query.elipType = param.type
    }

    // startDate <  endDate
    if (
      !_.isEmpty(param.startDate) &&
      !_.isEmpty(param.endDate) &&
      moment(param.endDate).isSameOrAfter(param.startDate)
    ) {
      query.createdAt = {
        $gte: moment(param.startDate),
        $lte: moment(param.endDate).add(1, 'd')
      }
    }

    // sortBy
    //      .sort({ createdAt: -1, vid: -1 })
    const sortObject = {}
    let sortBy = param.sortBy
    if(sortBy !== "createdAt"
       && sortBy !== "updatedAt") {
      sortBy = "createdAt"
    }
    sortObject[sortBy] = -1

    if (param.$or && query.$or) {
      query.$and = [{ $or: query.$or }, { $or: param.$or }]
    }

    if (param.$or && !query.$or) {
      query.$or = param.$or
    }

    const fields = 'vid title createdBy createdAt updatedAt status'
    const list = await db_elip
      .getDBInstance()
      .find(query, fields)
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
      .sort(sortObject)
      .limit(100)

    return list
  }
}
