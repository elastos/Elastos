import Base from './Base'
import * as _ from 'lodash'
import * as jwt from 'jsonwebtoken'
import { constant } from '../constant'
import { mail, logger, user as userUtil, utilCrypto } from '../utility'

export default class extends Base {
  private model: any
  protected init() {
    this.model = this.getDBModel('CVote')
  }

  public async update(param: any) {
    try {
      const { id, stage, message } = param
      const proposal = await this.model.findById(id)
      // generate jwt url
      const now = Math.floor(Date.now() / 1000)
      const jwtClaims = {
        iat: now,
        exp: now + 60 * 60 * 24,
        command: 'updatemilestone',
        iss: process.env.APP_DID,
        sid: proposal._id,
        callbackurl: `${process.env.API_URL}/api/milestone/owner-signature-callback`,
        data: {
          proposalhash: proposal.proposalHash,
          messagehash: utilCrypto.sha256D(message),
          stage: parseInt(stage),
          ownerpubkey: '',
          newownerpubkey: '',
          proposaltrackingtype: ''
        }
      }
      const jwtToken = jwt.sign(
        JSON.stringify(jwtClaims),
        process.env.APP_PRIVATE_KEY,
        {
          algorithm: 'ES256'
        }
      )
      // update withdraw history
      const url = `elastos://crproposal/${jwtToken}`
      return { success: true, url }
    } catch (error) {
      logger.error(error)
      return
    }
  }

  public async review(param: any) {
  }

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
