import Base from '../Base'
import SSOService from '../../service/SSOService'

// forum sso support
export default class SSO extends Base {
  // protected needLogin = true;
  async action() {
    const ssoService = this.buildService(SSOService)
    const url = await ssoService.login(this.getParam())

    return this.result(1, {
      url
    })
  }
}
