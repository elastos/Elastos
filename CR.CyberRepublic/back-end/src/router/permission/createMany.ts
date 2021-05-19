import Base from '../Base'
import Service from '../../service/PermissionService'

export default class extends Base {
  protected needLogin = true
  public async action() {
    const service = this.buildService(Service)

    const { permissions } = this.getParam()
    for (const perm of permissions) {
      await service.create(perm)
    }

    return this.result(1, { data: 'ok' })
  }
}

