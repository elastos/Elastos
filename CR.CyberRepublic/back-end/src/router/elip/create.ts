import Base from '../Base'
import ElipService from '../../service/ElipService'

export default class extends Base {
  protected needLogin = true
  async action() {
    const param = this.getParam()
    const service = this.buildService(ElipService)

    const rs = await service.create(param)
    return this.result(1, rs)
  }
}
