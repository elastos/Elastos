import Base from './Base'
import {Team} from './schema/TeamSchema'

export default class extends Base {
    protected getSchema(){
        return Team
    }
    protected getName(){
        return 'team'
    }
    protected rejectFields(){
        return {

        }
    }
}