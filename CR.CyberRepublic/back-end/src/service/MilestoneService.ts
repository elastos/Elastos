import Base from './Base'
import * as _ from 'lodash'
import { constant } from '../constant'
import { mail, logger, user as userUtil } from '../utility'

export default class extends Base {
  public async update(param: any) {
    try {
      console.log('param...', param)
    } catch (error) {
      logger.error(error)
      return
    }
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
