import Base from '../Base'
import ReleaseService from '../../service/ReleaseService'

export default class extends Base {
  protected needLogin = false

  public async action() {
    const service = this.buildService(ReleaseService)
    const param = this.getParam()

    const result = await service.show(param)

    return this.result(1, result)
  }
}
