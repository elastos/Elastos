import Base from './Base'
import { Release } from './schema/ReleaseSchema'

export default class extends Base {
    protected getSchema(){
        return Release
    }
    protected getName(){
        return 'release'
    }
}
