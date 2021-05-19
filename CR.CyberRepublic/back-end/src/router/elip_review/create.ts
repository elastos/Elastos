import Base from '../Base'
import ElipReviewService from '../../service/ElipReviewService'

export default class extends Base {
  protected needLogin = true
  async action() {
    const param = this.getParam()
    const service = this.buildService(ElipReviewService)

    const rs = await service.create(param)
    return this.result(1, rs)
  }
}
