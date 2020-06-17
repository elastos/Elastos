import Base from './Base'
import { Config } from './schema/ConfigSchema'

export default class extends Base {
    protected getSchema() {
        return Config
    }
    protected getName() {
        return 'config'
    }
}
