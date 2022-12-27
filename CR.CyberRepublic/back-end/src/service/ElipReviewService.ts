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
          constant.ELIP_STATUS.FINAL_REVIEW
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
        let content: { subject: string; body: string }
        if (isRejected) {
          elipStatus = constant.ELIP_STATUS.REJECTED
          content = this.rejectedMailTemplate(elip, review)
        }
        if (isApproved) {
          elipStatus = constant.ELIP_STATUS.DRAFT
          content = this.approvedMailTemplate(elip)
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
        
        const author = {
          name: userUtil.formatUsername(elip.createdBy),
          mail: elip.createdBy.email
        }
        this.notifyElipCreator(author, content)
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
        profile: { firstName: user.firstName, lastName: user.lastName },
        username: user.username
      }
      return { ...review._doc, createdBy, elipStatus }
    } catch (error) {
      logger.error(error)
      return
    }
  }

  private rejectedMailTemplate(elip: any, review: any) {
    const subject = 'ELIP Rejected'
    const body = `
    <p>CR secretary has marked your ELIP <${elip.title}> as Rejected, ID <#${elip.vid}>.</p>
    <p>${review.comment}<p>
    <p>Click this link to view more details:</p>
    <p><a href="${process.env.SERVER_URL}/elips/${elip._id}">${process.env.SERVER_URL}/elips/${elip._id}</a></p>
    <br />
    <p>Cyber Republic Team</p>
    <p>Thanks</p>
    `
    return { subject, body }
  }

  private approvedMailTemplate(elip: any) {
    const subject = 'ELIP Approved'
    const body = `
    <p>CR secretary has marked your ELIP <${elip.title}> as Approved, ID <#${elip.vid}>.</p>
    <p>Click this link to view more details:</p>
    <p><a href="${process.env.SERVER_URL}/elips/${elip._id}">${process.env.SERVER_URL}/elips/${elip._id}</a></p>
    <br />
    <p>Cyber Republic Team</p>
    <p>Thanks</p>
    `
    return { subject, body }
  }

  private proposedMailTemplate(elip: any, proposal: any) {
    const subject = `[ELIP marked as proposal] ELIP #${elip.vid} Marked as proposal`
    const body = `
    <p>Secretary has marked ELIP #${elip.vid} as proposal #${proposal.vid}.</p>
    <p>Click this link to view more details of the proposal:</p>
    <p><a href="${process.env.SERVER_URL}/proposals/${proposal._id}">${process.env.SERVER_URL}/proposals/${proposal._id}</a></p>
    <br />
    <p>Cyber Republic Team</p>
    <p>Thanks</p>
    `
    return { subject, body }
  }

  private async notifyElipCreator(
    author: { name: string; mail: string },
    content: { subject: string; body: string }
  ) {
    const mailObj = {
      to: author.mail,
      toName: author.name,
      subject: content.subject,
      body: content.body
    }

    mail.send(mailObj)
  }

  public async proposeElip(elip: any): Promise<Document> {
    const db_cvote = this.getDBModel('CVote')
    const db_elip = this.getDBModel('Elip')
    const vid = await this.getNewCVoteVid()

    const authorName = userUtil.formatUsername(elip.createdBy)
    const doc: any = {
      vid,
      type: constant.CVOTE_TYPE[elip.elipType],
      status: constant.CVOTE_STATUS.PROPOSED,
      published: true,
      proposedBy: authorName,
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
    const db_user = this.getDBModel('User')
    const councilMembers = await db_user.find({
      role: constant.USER_ROLE.COUNCIL
    })
    const voteResult = []
    _.each(councilMembers, user =>
      voteResult.push({
        votedBy: user._id,
        value: constant.CVOTE_RESULT.UNDECIDED
      })
    )
    doc.voteResult = voteResult
    doc.voteHistory = voteResult
    try {
      const proposal = await db_cvote.save(doc)
      await db_elip.update(
        { _id: elip._id },
        {
          reference: proposal._id,
          status: constant.ELIP_STATUS.SUBMITTED_AS_PROPOSAL
        }
      )
      const content = this.proposedMailTemplate(elip, proposal)
      const author = {
        name: authorName,
        mail: elip.createdBy.email
      }
      this.notifyElipCreator(author, content)
      this.notifyCouncilAfterPropose(proposal)
      return proposal
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
      <p>${cvote.title}</p>
      <p>Click this link to view more details:</p>
      <p><a href="${process.env.SERVER_URL}/proposals/${cvote._id}">${process.env.SERVER_URL}/proposals/${cvote._id}</a></p>
      <br />
      <p>Cyber Republic</p>
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
}
