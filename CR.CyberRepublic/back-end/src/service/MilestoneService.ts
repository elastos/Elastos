import Base from './Base'
import * as _ from 'lodash'
import * as jwt from 'jsonwebtoken'
import { constant } from '../constant'
import {
  mail,
  logger,
  user as userUtil,
  utilCrypto,
  getDidPublicKey,
  getPemPublicKey
} from '../utility'
const { WAITING_FOR_WITHDRAW, WAITING_FOR_APPROVAL } = constant.BUDGET_STATUS
const { ACTIVE } = constant.CVOTE_STATUS

export default class extends Base {
  private model: any
  protected init() {
    this.model = this.getDBModel('CVote')
  }

  public async update(param: any) {
    try {
      const { id, milestoneKey, message } = param
      if (!message) {
        return { success: false }
      }
      const proposal = await this.model.findById(id)
      // check if current user is the proposal's owner
      if (!proposal.proposer.equals(this.currentUser._id)) {
        return { success: false }
      }
      const status = proposal.status
      if (status !== ACTIVE) {
        return { success: false }
      }
      // check if milestoneKey is valid
      const budget = proposal.budget.filter(
        (item: any) => item.milestoneKey === milestoneKey
      )
      if (_.isEmpty(budget)) {
        return { success: false }
      }
      if (budget.status !== WAITING_FOR_WITHDRAW) {
        return { success: false }
      }

      const now = Math.floor(Date.now() / 1000)
      const hashMsg = {
        date: now,
        message
      }
      const messageHash = utilCrypto.sha256D(JSON.stringify(hashMsg))
      // update withdrawal history
      const history = { message, milestoneKey, messageHash }
      await this.model.update(
        { _id: id },
        { $push: { withdrawalHistory: history } }
      )

      const ownerPublicKey = _.get(this.currentUser, 'did.compressedPublicKey')
      // generate jwt url
      const jwtClaims = {
        iat: now,
        exp: now + 60 * 60 * 24,
        command: 'updatemilestone',
        iss: process.env.APP_DID,
        callbackurl: `${process.env.API_URL}/api/proposals/milestones/signature-callback`,
        data: {
          proposalhash: proposal.proposalHash,
          messagehash: messageHash,
          stage: parseInt(milestoneKey),
          ownerpubkey: ownerPublicKey,
          newownerpubkey: '',
          proposaltrackingtype: status
        }
      }
      const jwtToken = jwt.sign(
        JSON.stringify(jwtClaims),
        process.env.APP_PRIVATE_KEY,
        { algorithm: 'ES256' }
      )

      const url = `elastos://crproposal/${jwtToken}`
      return { success: true, url }
    } catch (error) {
      logger.error(error)
      return
    }
  }

  public async ownerSignatureCallback(param: any) {
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
        claims.req.slice('elastos://crproposal/'.length)
      )
      const proposalHash = _.get(payload, 'data.proposalhash')
      const messageHash = _.get(payload, 'data.messagehash')
      if (!proposalHash || !messageHash) {
        return {
          code: 400,
          success: false,
          message: 'Problems parsing jwt token of CR website.'
        }
      }

      const proposal = await this.model.findOne({ proposalHash })
      if (!proposal) {
        return {
          code: 400,
          success: false,
          message: 'There is no this proposal.'
        }
      }
      const ownerPublicKey = _.get(proposal, 'ownerPublicKey')
      if (!ownerPublicKey) {
        return {
          code: 400,
          success: false,
          message: `Can not get your did's public key.`
        }
      }
      const pemPublicKey = getPemPublicKey(ownerPublicKey)
      // verify response data from ela wallet
      return jwt.verify(
        jwtToken,
        pemPublicKey,
        async (err: any, decoded: any) => {
          if (err) {
            return {
              code: 401,
              success: false,
              message: 'Verify signatrue failed.'
            }
          } else {
            try {
              await this.model.update(
                { proposalHash, 'withdrawalHistory.messageHash': messageHash },
                {
                  $set: {
                    'withdrawalHistory.$.signature': decoded.data,
                    'budget.$.status': WAITING_FOR_APPROVAL
                  }
                }
              )
              this.notifySecretaries(this.updateMailTemplate(proposal.vid))
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
    } catch (err) {
      logger.error(err)
      return {
        code: 500,
        success: false,
        message: 'Something went wrong'
      }
    }
  }

  public async checkSignature(param: any) {
    const { id, messageHash } = param
    const proposal: any = await this.model.findById(id)
    if (proposal) {
      const history = proposal.withdrawalHistory.filter(
        (item) => item.messageHash === messageHash
      )
      if (_.isEmpty(history)) {
        return { success: false }
      }
      if (_.get(history, 'signature')) {
        return { success: true }
      }
    } else {
      return { success: false }
    }
  }

  public async review(param: any) {}

  private updateMailTemplate(id: string) {
    const subject =
      '【Payment Review】One payment request is waiting for your review'
    const body = `
      <p>One payment request in proposal #${id} is waiting for your review:</p>
      <p>Click this link to view more details:</p>
      <p><a href="${process.env.SERVER_URL}/proposals/${id}">${process.env.SERVER_URL}/proposals/${id}</a></p>
      <br />
      <p>Cyber Republic Team</p>
      <p>Thanks</p>
    `
    return { subject, body }
  }

  private reviewMailTemplate() {}

  private async notifySecretaries(content: { subject: string; body: string }) {
    const db_user = this.getDBModel('User')
    const currentUserId = _.get(this.currentUser, '_id')
    const secretaries = await db_user.find({
      role: constant.USER_ROLE.SECRETARY
    })
    const toUsers = _.filter(
      secretaries,
      (user) => !user._id.equals(currentUserId)
    )
    const toMails = _.map(toUsers, 'email')

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
      subject: content.subject,
      body: content.body,
      recVariables
    }

    mail.send(mailObj)
  }
}
