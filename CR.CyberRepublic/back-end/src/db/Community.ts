import Base from './Base'
import {Community} from './schema/CommunitySchema'

export default class extends Base {
    protected getSchema(){
        return Community
    }
    protected getName(){
        return 'community'
    }
    protected rejectFields(){
        return {

        }
    }
}
