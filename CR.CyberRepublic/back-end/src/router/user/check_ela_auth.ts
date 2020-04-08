import Base from '../Base'
import UserService from '../../service/UserService'
import { utilCrypto } from '../../utility'
import * as moment from 'moment'

export default class extends Base {
  async action() {
    const userService = this.buildService(UserService)
    const rs: any = await userService.checkElaAuth(this.getParam())
    if (rs && rs.success && rs.did) {
      const user: any = await userService.findUserByDid(rs.did)
      if (user) {
        const resultData = { user }
        // record user login date
        userService.recordLogin({ userId: user.id })
  
        // always return api-token on login, this is needed for future requests
        this.session.userId = user.id
        resultData['api-token'] = utilCrypto.encrypt(
          JSON.stringify({
            userId: user.id,
            expired: moment().add(30, 'd').unix()
          })
        )
        return this.result(1, resultData)
      }
    }
    return this.result(1, rs)
  }
}
