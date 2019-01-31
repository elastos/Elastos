import Base from '../Base'
import Service from '../../service/PermissionRoleService'

export default class extends Base {
  protected needLogin = true

  public async action() {
    const service = this.buildService(Service)
    const param = this.getParam()

    const result = await service.delete(param)

    return this.result(1, result)
  }
}
