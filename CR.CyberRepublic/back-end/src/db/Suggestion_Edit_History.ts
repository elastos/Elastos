import Base from './Base'
import { SuggestionEditHistory } from './schema/SuggestionEditHistorySchema'

export default class extends Base {
  protected getSchema() {
    return SuggestionEditHistory
  }
  protected getName() {
    return 'suggestion_edit_history'
  }
}
