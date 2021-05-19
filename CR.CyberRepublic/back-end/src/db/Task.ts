import Base from './Base'
import {Task} from './schema/TaskSchema'

export default class extends Base {
    protected getSchema(){
        return Task
    }
    protected getName(){
        return 'task'
    }
}
