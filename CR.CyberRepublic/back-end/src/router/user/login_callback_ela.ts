import Base from '../Base'
import UserService from '../../service/UserService'

export default class extends Base {
  async action() {
    const userService = this.buildService(UserService)
    const rs = await userService.loginCallbackEla(this.getParam())
    return this.result(1, rs)
  }
}
