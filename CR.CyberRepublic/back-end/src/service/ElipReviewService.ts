import Base from './Base'
import { Document } from 'mongoose'
import { constant } from '../constant'
import { mail, logger } from '../utility'

export default class extends Base {
  public async create(param: any): Promise<Document> {
    try {
      const db_elip_review = this.getDBModel('Elip_Review')
      const db_elip = this.getDBModel('Elip')
      const { comment, status, elipId } = param
      const elip = await db_elip
        .getDBInstance()
        .findOne({ _id: elipId })
      if (!elip) {
        throw 'ElipReviewService.create - invalid elip id'
      }
      const doc: any = {
        comment,
        status,
        createdBy: this.currentUser._id,
        elipId
      }
      const review = await db_elip_review.save(doc)
      await db_elip.update({ _id: elipId }, { status })
      this.notifyElipCreator(review, elip, status)
      return { ...review._doc, createdBy: this.currentUser}
    } catch (error) {
      logger.error(error)
      return
    }
  }

  private async notifyElipCreator(review: any, elip: any, status: string) {
    const rejected = status === constant.ELIP_REVIEW_STATUS.REJECTED
    const subject = rejected ? 'ELIP Rejected' : 'ELIP Approved'
    const body = `
      <p>CR secretary has marked your ELIP <${elip.title}> as "${rejected ? 'Rejected' : 'Approved'}", ID <${elip.vid}>.</p>
      <br />
      ${rejected ? `<p>${review}<p>` : ''}
      <br />
      <p>Click this link to view more details: <a href="${
      process.env.SERVER_URL
      }/elips/${elip._id}">${process.env.SERVER_URL}/elips/${elip._id}</a></p>
      <br /> <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `

    const creator = this.getDBModel('User')
      .getDBInstance()
      .findOne({ _id: elip.createdBy })

    const mailObj = {
      to: creator.email,
      subject,
      body
    }

    mail.send(mailObj)
  }
}
