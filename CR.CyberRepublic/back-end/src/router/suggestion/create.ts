import Base from '../Base'
import SuggestionService from '../../service/SuggestionService'

export default class extends Base {
  protected needLogin = true
  public async action() {
    const service = this.buildService(SuggestionService)
    const rs = await service.create(this.getParam())
    return this.result(1, rs)
  }
}

