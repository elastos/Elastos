import Base from './Base'
import { Did } from './schema/DidSchema'

export default class extends Base {
    protected getSchema(){
        return Did
    }
    protected getName(){
        return 'dids'
    }
}
