import Base from './Base'
import { CVote } from './schema/CVoteSchema'

export default class extends Base {
  protected getSchema() {
    return CVote
  }
  protected getName() {
    return 'cvote'
  }
  protected buildSchema() {
    const schema = super.buildSchema()
    schema.index({ published: -1, vid: -1 })
    schema.index({ vid: -1 })
    return schema
  }
}
