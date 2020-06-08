import Base from '../Base'
import SuggestionService from '../../service/SuggestionService'

export default class extends Base {
  async action() {
    const suggestionService = this.buildService(SuggestionService)
    const rs = await suggestionService.councilMemberSignatureCallback(
      this.getParam()
    )
    return this.result(1, rs)
  }
}
