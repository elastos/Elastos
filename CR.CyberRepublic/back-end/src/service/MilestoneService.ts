import Base from './Base'
import * as _ from 'lodash'
import * as jwt from 'jsonwebtoken'
import { constant } from '../constant'
import {
  mail,
  logger,
  user as userUtil,
  utilCrypto,
  getDidPublicKey
} from '../utility'
const { WAITING_FOR_WITHDRAW } = constant.BUDGET_STATUS

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
      const status = proposal.status
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

      const rs: any = await getDidPublicKey(claims.iss)
      if (!rs) {
        return {
          code: 400,
          success: false,
          message: `Can not get your did's public key.`
        }
      }

      // verify response data from ela wallet
      return jwt.verify(
        jwtToken,
        rs.publicKey,
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
                { $set: { 'withdrawalHistory.$.signature': decoded.data } }
              )
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

  public async review(param: any) {}

  private updateMailTemplate() {}

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
