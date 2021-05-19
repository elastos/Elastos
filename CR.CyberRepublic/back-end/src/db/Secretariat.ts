import Base from './Base'
import { Secretariat } from './schema/SecretariatSchema'

export default class extends Base {
    protected getSchema() {
        return Secretariat
    }
    protected getName() {
        return 'secretariat'
    }
}
