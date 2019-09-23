import Base from './Base'
import { Elip } from './schema/ElipSchema'

export default class extends Base {
  protected getSchema() {
    return Elip
  }
  protected getName() {
    return 'elip'
  }
  protected buildSchema() {
    const schema = super.buildSchema()
    schema.index({ status: -1, vid: -1 })
    return schema
  }
}
