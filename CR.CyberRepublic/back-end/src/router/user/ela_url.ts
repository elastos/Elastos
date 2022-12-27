import Base from '../Base'
import UserService from '../../service/UserService'

export default class extends Base {
  protected needLogin = true
  async action() {
    const userService = this.buildService(UserService)
    const rs = await userService.getElaUrl()
    return this.result(1, rs)
  }
}
