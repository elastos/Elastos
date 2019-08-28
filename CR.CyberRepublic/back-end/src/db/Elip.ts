import Base from './Base'
import { Elip } from './schema/ElipSchema'

export default class extends Base {
  protected getSchema() {
    return Elip
  }
  protected getName() {
    return 'elip'
  }
  protected rejectFields() {
    return {}
  }
}
