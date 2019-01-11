import Base from '../Base'
import SuggestionService from '../../service/SuggestionService'

export default class extends Base {
  protected needLogin = false

  public async action() {
    const service = this.buildService(SuggestionService)
    const param = this.getParam()

    const result = await service.show(param)

    return this.result(1, result)
  }
}
