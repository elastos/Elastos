import Base from './Base'
import { Document } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../constant'
import { permissions } from '../utility'
import * as moment from 'moment'
import { mail, user as userUtil } from '../utility'

interface Mail {
  subject: string;
  body: string;
  role?: string;
  user?: object;
}

export default class extends Base {
  private model: any
  protected init() {
    this.model = this.getDBModel('CVote_Summary')
  }

  public async create(param: any): Promise<Document> {
    const {
      content, proposalId, status
    } = param

    const doc: any = {
      content,
      proposalId,
      status: constant.CVOTE_SUMMARY_STATUS.REVIEWING,
      createdBy: this.currentUser._id
    }
    if (status === constant.CVOTE_SUMMARY_STATUS.DRAFT) {
      doc.status = status
    }

    try {
      const rs = await this.model.save(doc)
      if (rs.status === constant.CVOTE_SUMMARY_STATUS.REVIEWING) {
        const proposal = await this.getProposalById(proposalId)
        if (proposal) this.notifySecretary(proposal)
      }
      return rs
    } catch (error) {
      console.log(error)
      return
    }
  }

  /**
   * @param param
   * @returns {Promise<"mongoose".Document>}
   */
  public async update(param: any): Promise<Document> {
    const {
      id: _id, content, status
    } = param

    if (!this.currentUser || !this.currentUser._id) {
      throw 'CVoteSummaryService.updateDraft - invalid current user'
    }

    const cur = await this.model.findOne({ _id })
    if (!cur) {
      throw 'CVoteSummaryService.updateDraft - invalid id'
    }

    if (cur.status !== constant.CVOTE_SUMMARY_STATUS.DRAFT) {
      throw 'CVoteSummaryService.updateDraft - only DRAFT can be updated'
    }

    if (!this.isOwner(cur)) {
      throw 'CVoteSummaryService.updateDraft - not owner'
    }

    const doc: any = {
      content
    }

    if (status && _.includes([constant.CVOTE_SUMMARY_STATUS.DRAFT, constant.CVOTE_SUMMARY_STATUS.REVIEWING], status)) {
      doc.status = status
    }

    try {
      await this.model.update({ _id }, doc)
      if (status === constant.CVOTE_SUMMARY_STATUS.REVIEWING) {
        const proposal = await this.getProposalById(cur.proposalId)
        if (proposal) this.notifySecretary(proposal)
      }

      return await this.getById(_id)
    } catch (error) {
      console.log('error happened: ', error)
      return
    }
  }

  /**
   * list all including unpublished, should only for AUTHORIZED peopole
   * @param query
   * @returns {Promise<Object>}
   */
  public async list(param: any): Promise<Object> {
    const { proposalId } = param
    const proposal = await this.getProposalById(proposalId)
    if (!proposalId || !proposal) {
      throw 'CVoteSummaryService.list - invalid proposal'
    }
    const query: any = {
      proposalId,
    }

    const cursor = this.model.getDBInstance().find(query)
      .populate('comment.createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
      .sort({ createdAt: 1 })
    const totalCursor = this.model.getDBInstance().find(query).count()

    const list = await cursor
    const total = await totalCursor

    return {
      list,
      total
    }
  }

  /**
   * list only public
   * @param query
   * @returns {Promise<Object>}
   */
  public async listPublic(param: any): Promise<Object> {
    const { proposalId } = param
    if (!proposalId) {
      throw 'CVoteSummaryService.list - must specify a proposal id'
    }

    const query: any = {
      proposalId,
      status: constant.CVOTE_SUMMARY_STATUS.PUBLISHED,
    }

    const cursor = this.model.getDBInstance().find(query).sort({
      createdAt: 1
    })
    const totalCursor = this.model.getDBInstance().find(query).count()

    const list = await cursor
    const total = await totalCursor

    return {
      list,
      total,
    }
  }

  public async approve(param: any): Promise<any> {
    const { id } = param
    const cur = await this.getById(id)
    const createdBy = _.get(this.currentUser, '_id')

    if (!cur) {
      throw 'invalid id'
    }

    if (cur.status !== constant.CVOTE_SUMMARY_STATUS.REVIEWING) {
      throw 'CVoteSummaryService.updateDraft - only REVIEWING can be updated'
    }

    await this.model.update({ _id: id }, {
      $set: {
        status: constant.CVOTE_SUMMARY_STATUS.PUBLISHED,
        comment: {
          createdBy,
        }
      }
    })

    const proposal = await this.getProposalById(cur.proposalId)
    if (proposal) this.notifyApproved(proposal)

    return await this.getById(id)
  }

  public async reject(param: any): Promise<any> {
    const { id, comment } = param
    const cur = await this.getById(id)
    const createdBy = _.get(this.currentUser, '_id')

    if (!cur) {
      throw 'CVoteSummaryService.reject - invalid id'
    }

    if (cur.status !== constant.CVOTE_SUMMARY_STATUS.REVIEWING) {
      throw 'CVoteSummaryService.reject - only REVIEWING can be updated'
    }

    if (!comment) {
      throw 'CVoteSummaryService.reject - comment is required'
    }

    await this.model.update({ _id: id }, {
      $set: {
        status: constant.CVOTE_SUMMARY_STATUS.REJECT,
        comment: {
          content: comment,
          createdBy,
        }
      }
    })

    const proposal = await this.getProposalById(cur.proposalId)
    if (proposal) this.notifyRejected(proposal)

    return await this.getById(id)
  }

  public async getById(id): Promise<any> {
    return await this.model.getDBInstance().findOne({ _id: id }, '-voteHistory')
  }

  public async getProposalById(id): Promise<any> {
    return await this.getDBModel('CVote').getDBInstance().findOne({ _id: id }, '-voteHistory')
  }

  private async notifyUsers(param: Mail) {
    const { subject, body, user, role } = param
    const db_user = this.getDBModel('User')
    const currentUserId = _.get(this.currentUser, '_id')
    const receivers = user ? [user] : await db_user.find({ role })
    const toUsers = _.filter(receivers, user => (user._id && user._id.toString()) !== currentUserId)
    const toMails = _.map(toUsers, 'email')

    const recVariables = _.zipObject(toMails, _.map(toUsers, (user) => ({
      _id: user._id,
      username: userUtil.formatUsername(user)
    })))

    const mailObj = {
      to: toMails,
      // toName: ownerToName,
      subject,
      body,
      recVariables,
    }

    mail.send(mailObj)
  }

  private async notifySecretary(cvote: any) {
    const subject = `[Review needed] Summary is updated in Proposal #${cvote.vid}`
    const body = `
      <p>${cvote.proposedBy} has updated the summary in proposal #${cvote.vid}</p>
      <br />
      <p>Click this link to view more details: <a href="${process.env.SERVER_URL}/proposals/${cvote._id}">${process.env.SERVER_URL}/proposals/${cvote._id}</a></p>
      <br /> <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
    `

    const mailObj = {
      subject,
      body,
      role: constant.USER_ROLE.SECRETARY,
    }

    this.notifyUsers(mailObj)
  }

  private async notifyRejected(cvote: any) {
    const subject = `[Summary Rejected] Please respond in Proposal #${cvote.vid}`
    const body = `
      <p>Your summary update has been rejected in proposal #${cvote.vid}</p>
      <br />
      <p>Click this link to view more details: <a href="${process.env.SERVER_URL}/proposals/${cvote._id}">${process.env.SERVER_URL}/proposals/${cvote._id}</a></p>
      <br /> <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
    `
    const user = await this.getDBModel('User').getDBInstance().findOne({ _id: cvote.proposer })

    const mailObj = {
      subject,
      body,
      user,
    }

    this.notifyUsers(mailObj)
  }

  private async notifyApproved(cvote: any) {
    const subject = `[Summary approved] Approved in Proposal #${cvote.vid}`
    const body = `
      <p>Your summary update has been approved and published in proposal #${cvote.vid}</p>
      <br />
      <p>Click this link to view more details: <a href="${process.env.SERVER_URL}/proposals/${cvote._id}">${process.env.SERVER_URL}/proposals/${cvote._id}</a></p>
      <br /> <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
    `
    const user = await this.getDBModel('User').getDBInstance().findOne({ _id: cvote.proposer })

    const mailObj = {
      subject,
      body,
      user,
    }

    this.notifyUsers(mailObj)
  }

  private isOwner(doc) {
    const userId = _.get(this.currentUser, '_id', '')
    return userId.toString() === _.get(doc, 'createdBy').toString()
  }
}
