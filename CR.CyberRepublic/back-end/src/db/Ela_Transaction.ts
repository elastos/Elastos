import Base from './Base'
import { ElaTransaction } from './schema/ElaTransactionSchema'

export default class extends Base {
    protected getSchema() {
        return ElaTransaction
    }
    protected getName() {
        return 'ela_transaction'
    }
}
