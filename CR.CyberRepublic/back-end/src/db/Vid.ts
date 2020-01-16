import Base from './Base'
import { Vid } from './schema/VidSchema'

export default class extends Base {
  protected getSchema() {
    return Vid
  }
  protected getName() {
    return 'vid'
  }
  protected buildSchema() {
    const schema = super.buildSchema()
    
    schema.index({ tableName: -1, vid: -1 })
    return schema
  }
}
