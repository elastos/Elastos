import Base from '../Base'
import ReleaseService from '../../service/ReleaseService'

export default class extends Base {
  protected needLogin = true

  public async action() {
    const service = this.buildService(ReleaseService)
    const param = this.getParam()

    const result = await service.update(param)

    return this.result(1, result)
  }
}
