import Base from '../Base'
import ElipService from '../../service/ElipService'

export default class extends Base {
  async action() {
    const param = this.getParam()
    const service = this.buildService(ElipService)

    const id = param.id
    const rs = await service.getById(id)
    return this.result(1, rs)
  }
}
