import Base from './Base';
import { Suggestion } from './schema/SuggestionSchema';
import { autoIncrement } from 'mongoose-plugin-autoinc';

export default class extends Base {
  protected getSchema() {
    return Suggestion;
  }
  protected getName() {
    return 'suggestion'
  }
  protected buildSchema() {
    const schema = super.buildSchema();
    const options = {
      model: this.getName(),
      field: 'displayId',
      startAt: 1,
    }
    schema.plugin(autoIncrement, options);
    return schema
  }
}
