import Base from '../Base'
import Service from '../../service/PermissionRoleService'

export default class extends Base {
  protected needLogin = true
  public async action() {
    const service = this.buildService(Service)
    const rs = await service.create(this.getParam())
    console.log('create,,,....')
    return this.result(1, rs)
  }
}

