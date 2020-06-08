import Base from '../Base'
import SuggestionService from '../../service/SuggestionService'

export default class extends Base {
  protected needLogin = true
  async action() {
    const suggestionService = this.buildService(SuggestionService)
    const rs = await suggestionService.getSignatureUrl(this.getParam())
    return this.result(1, rs)
  }
}
