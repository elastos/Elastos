import Base from '../Base'
import CVoteTrackingService from '../../service/CVoteTrackingService'

export default class extends Base {
  protected needLogin = true
  async action() {
    const param = this.getParam()
    const service = this.buildService(CVoteTrackingService)

    const rs = await service.update(param)
    return this.result(1, rs)
  }
}