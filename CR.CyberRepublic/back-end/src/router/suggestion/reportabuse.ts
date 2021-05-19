import Base from '../Base'
import SuggestionService from '../../service/SuggestionService'

export default class extends Base {
  protected needLogin = true

  public async action() {
    const service = this.buildService(SuggestionService)
    const param = this.getParam()

    const result = await service.reportabuse(param)

    return this.result(1, result)
  }
}
