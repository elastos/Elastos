import Base from './Base'
import {User_Team} from './schema/TeamSchema'

export default class extends Base {
    protected getSchema(){
        return User_Team
    }
    protected getName(){
        return 'user_team'
    }
}