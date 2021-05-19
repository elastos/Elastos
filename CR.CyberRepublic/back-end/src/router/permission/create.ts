import Base from '../Base'
import Service from '../../service/PermissionService'

export default class extends Base {
  protected needLogin = true
  public async action() {
    const service = this.buildService(Service)
    const rs = await service.create(this.getParam())

    return this.result(1, rs)
  }
}

