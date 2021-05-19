import Base from '../Base'
import ElipService from '../../service/ElipService'

export default class extends Base {
  protected needLogin = true
  async action() {
    const param = this.getParam()
    const service = this.buildService(ElipService)

    const id = param.id
    const rs = await service.remove(id)
    return this.result(1, rs)
  }
}
