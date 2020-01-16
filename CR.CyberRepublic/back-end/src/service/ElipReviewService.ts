import * as _ from 'lodash'
import { Document } from 'mongoose'
import Base from './Base'
import { constant } from '../constant'
import { mail, logger, user as userUtil } from '../utility'

export default class extends Base {
  public async getVid() {
    const db_elip = this.getDBModel('Elip')

    // get last vid
    let lastId: Number = 1

    const elip = await db_elip
      .getDBInstance()
      .find({
        status: {
          $in: [
            constant.ELIP_STATUS.DRAFT,
            constant.ELIP_STATUS.CANCELLED,
            constant.ELIP_STATUS.FINAL_REVIEW,
            constant.ELIP_STATUS.SUBMITTED_AS_PROPOSAL
          ]
        }
      })
      .sort({"vid": -1})
      .limit(1)
    if(elip.length > 0) {
      lastId = elip[0].vid + 1
    }

    // get Vid last id
    const db_vid = this.getDBModel('Vid')
    const elip_vid = await db_vid
      .getDBInstance()
      .findOne({tableName: "elip"})

    if(!elip_vid) {
      await db_vid
        .getDBInstance()
        .save({tableName: "elip", vid: lastId})
    }else{
      const elip_vid = await db_vid
        .getDBInstance()
        .findOneAndUpdate({tableName: "elip"}, {$inc: {vid: 1}})
      lastId = elip_vid.vid + 1
    }

    return lastId
  }

  public async create(param: any): Promise<Document> {
    try {
      const db_elip_review = this.getDBModel('Elip_Review')
      const db_elip = this.getDBModel('Elip')
      const { elipId } = param
      const elip = await db_elip
        .getDBInstance()
        .findById({ _id: elipId })
        .populate('createdBy')
      if (!elip) {
        throw 'ElipReviewService.create - invalid elip id'
      }
      if (
        ![
          constant.ELIP_STATUS.WAIT_FOR_REVIEW,
          constant.ELIP_STATUS.FINAL_REVIEW,
        ].includes(elip.status)
      ) {
        throw 'ElipReviewService.create - can not review this elip'
      }

      const user = this.currentUser
      const { comment, status } = param
      const doc: any = {
        comment,
        status,
        createdBy: user._id,
        elipId
      }
      const review = await db_elip_review.save(doc)
      const isRejected = status === constant.ELIP_REVIEW_STATUS.REJECTED
      const isApproved = status === constant.ELIP_REVIEW_STATUS.APPROVED
      let elipStatus: string
      if (elip.status === constant.ELIP_STATUS.WAIT_FOR_REVIEW) {
        if (isRejected) {
          elipStatus = constant.ELIP_STATUS.REJECTED
        }
        if (isApproved) {
          elipStatus = constant.ELIP_STATUS.DRAFT
        }

        const doc: any = { status: elipStatus }
        if (
          elipStatus === constant.ELIP_STATUS.DRAFT &&
          elip.status !== constant.ELIP_STATUS.DRAFT
        ) {
          doc.vid = await this.getVid()
        }
        
        await db_elip.update(
          { _id: elipId },
          doc
        )
        this.notifyElipCreator(review, elip, status)
      }
      if (elip.status === constant.ELIP_STATUS.FINAL_REVIEW) {
        if (isRejected) {
          elipStatus = constant.ELIP_STATUS.DRAFT

          const doc: any = { status: elipStatus }
          if (
            elipStatus === constant.ELIP_STATUS.DRAFT &&
            elip.status !== constant.ELIP_STATUS.DRAFT
          ) {
            doc.vid = await this.getVid()
          }
          
          await db_elip.update(
            { _id: elipId },
            doc
          )
        }
        if (isApproved) {
          elipStatus = constant.ELIP_STATUS.SUBMITTED_AS_PROPOSAL
          await this.proposeElip(elip)
        }
      }

      const createdBy = {
        _id: user._id,
        profile: { firstName: user.firstName, lastName: user.lastName},
        username: user.username
      }
      return { ...review._doc, createdBy, elipStatus }
    } catch (error) {
      logger.error(error)
      return
    }
  }

  private async notifyElipCreator(review: any, elip: any, status: string) {
    const rejected = status === constant.ELIP_REVIEW_STATUS.REJECTED
    const subject = rejected ? 'ELIP Rejected' : 'ELIP Approved'
    const body = `
      <p>CR secretary has marked your ELIP <${elip.title}> as "${rejected ? 'Rejected' : 'Approved'}", ID <#${elip.vid}>.</p>
      <br />
      ${rejected ? `<p>${review.comment}<p>` : ''}
      <br />
      <p>Click this link to view more details: <a href="${
      process.env.SERVER_URL
      }/elips/${elip._id}">${process.env.SERVER_URL}/elips/${elip._id}</a></p>
      <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `

    const mailObj = {
      to: elip.createdBy.email,
      toName: userUtil.formatUsername(elip.createdBy),
      subject,
      body
    }

    mail.send(mailObj)
  }

  public async proposeElip(elip: any): Promise<Document> {
    const db_cvote = this.getDBModel('CVote')
    const db_elip = this.getDBModel('Elip')
    
    const vid = await this.getNewCVoteVid()
    const doc: any = {
      vid,
      type: constant.CVOTE_TYPE[elip.elipType],
      status: constant.CVOTE_STATUS.PROPOSED,
      published: true,
      proposedBy: userUtil.formatUsername(elip.createdBy),
      proposer: elip.createdBy._id,
      createdBy: this.currentUser._id,
      referenceElip: elip._id,
      proposedAt: Date.now()
    }

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
    Object.assign(doc, _.pick(elip, BASE_FIELDS))
    try {
      const res = await db_cvote.save(doc)
      await db_elip.update(
        { _id: elip._id },
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

  public async getNewCVoteVid() {
    const db_cvote = this.getDBModel('CVote')
    const n = await db_cvote.count({})
    return n + 1
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
      subject,
      body,
      recVariables
    }

    mail.send(mailObj)
  }
}
