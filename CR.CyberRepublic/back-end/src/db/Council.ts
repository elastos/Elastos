import Base from './Base'
import { Council } from './schema/CouncilSchema'

export default class extends Base {
    protected getSchema() {
        return Council
    }
    protected getName() {
        return 'council'
    }
}
