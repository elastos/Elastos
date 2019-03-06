import Base from '../Base'
import GoogleService from '../../service/GoogleService'

export default class extends Base {
  protected needLogin = true

  public async action() {
    const service = this.buildService(GoogleService)
    const param = this.getParam()

    const result = await service.translate(param)

    return this.result(1, result)
  }
}
