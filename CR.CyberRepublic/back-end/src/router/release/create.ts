import Base from '../Base'
import ReleaseService from '../../service/ReleaseService'

export default class extends Base {
  protected needLogin = true
  public async action() {
    const service = this.buildService(ReleaseService)
    const rs = await service.create(this.getParam())
    return this.result(1, rs)
  }
}

