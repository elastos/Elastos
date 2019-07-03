import Base from '../Base'
import CVoteSummaryService from '../../service/CVoteSummaryService'

export default class extends Base {
  protected needLogin = true
  async action() {
    const param = this.getParam()
    const service = this.buildService(CVoteSummaryService)

    const rs = await service.update(param)
    return this.result(1, rs)
  }
}