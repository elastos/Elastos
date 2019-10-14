import Base from './Base'
import { Elip_Review } from './schema/ElipReviewSchema'

export default class extends Base {
  protected getSchema() {
    return Elip_Review
  }
  protected getName() {
    return 'elip_review'
  }
}
