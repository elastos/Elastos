import Base from './Base'
import { Document } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../constant'
import { mail, logger, user as userUtil } from '../utility'

export default class extends Base {
  public async update(param: any): Promise<Document> {
    try {
      const db_elip = this.getDBModel('Elip')
      const { _id, status } = param
      const elip = await db_elip
        .getDBInstance()
        .findOne({ _id })
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

      const { title, description } = param
      const doc: any = {}
      if (title) {
        doc.title = title
      }
      if (description) {
        doc.description = description
      }
      if (doc.title || doc.description) {
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
      const { title,
              elipType,
              description,
              specifications,
              motivation,
              rationale,
              backwardCompatibility,
              copyright
            } = param
      if(!constant.ELIP_STATUS[elipType]){
        elipType = _.values(constant.ELIP_STATUS)[0]
      }
      const vid = await this.getNewVid()
      const doc: any = {
        title,
        vid,
        elipType,
        description,
        specifications,
        motivation,
        rationale,
        backwardCompatibility,
        copyright,
        status: constant.ELIP_STATUS.WAIT_FOR_REVIEW,
        contentType: constant.CONTENT_TYPE.MARKDOWN,
        createdBy: this.currentUser._id
      }
      const elip = await db_elip.save(doc)
      this.notifySecretaries(elip)
      return elip
    } catch (error) {
      logger.error(error)
      return
    }
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

  public async getById(id: string): Promise<any> {
    const db_elip = this.getDBModel('Elip')
    const rs = await db_elip
      .getDBInstance()
      .findById({ _id: id })
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
}
