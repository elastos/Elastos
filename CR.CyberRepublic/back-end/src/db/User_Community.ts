import Base from './Base'
import {User_Community} from './schema/CommunitySchema'

export default class extends Base {
    protected getSchema(){
        return User_Community
    }
    protected getName(){
        return 'user_community'
    }
}
