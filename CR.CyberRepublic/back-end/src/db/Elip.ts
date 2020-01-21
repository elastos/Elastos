import Base from './Base'
import { Elip } from './schema/ElipSchema'
// import { autoIncrement } from 'mongoose-plugin-autoinc'

export default class extends Base {
  protected getSchema() {
    return Elip
  }
  protected getName() {
    return 'elip'
  }
  protected buildSchema() {
    const schema = super.buildSchema()
    const options = {
      model: this.getName(),
      field: 'vid',
      startAt: 10,
    }
    // schema.plugin(autoIncrement, options)
    // schema.index({ status: -1, vid: -1 })
    return schema
  }
}
