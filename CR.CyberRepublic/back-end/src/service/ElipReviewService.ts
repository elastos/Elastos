import * as _ from 'lodash'
import { Document } from 'mongoose'
import Base from './Base'
import { constant } from '../constant'
import { mail, logger, user as userUtil } from '../utility'

export default class extends Base {
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
      const user = this.currentUser
      const { comment, status } = param
      const doc: any = {
        comment,
        status,
        createdBy: user._id,
        elipId
      }
      const review = await db_elip_review.save(doc)
      const elipStatus = status === constant.ELIP_REVIEW_STATUS.REJECTED ? constant.ELIP_STATUS.REJECTED : constant.ELIP_STATUS.DRAFT
      await db_elip.update(
        { _id: elipId },
        { status: elipStatus }
      )
      this.notifyElipCreator(review, elip, status)
    
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
      
    console.log(elip)

    const mailObj = {
      to: elip.createdBy.email,
      toName: userUtil.formatUsername(elip.createdBy),
      subject,
      body
    }

    mail.send(mailObj)
  }
}
