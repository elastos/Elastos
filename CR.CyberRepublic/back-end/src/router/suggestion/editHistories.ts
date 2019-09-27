import Base from '../Base'
import SuggestionService from '../../service/SuggestionService'

export default class extends Base {
  public async action() {
    const service = this.buildService(SuggestionService)
    const param = this.getParam()

    const result = await service.editHistories(param)

    return this.result(1, result)
  }
}
