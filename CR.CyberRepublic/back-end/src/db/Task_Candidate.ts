import Base from './Base'
import {Task_Candidate} from './schema/TaskSchema'

export default class extends Base {
    protected getSchema(){
        return Task_Candidate
    }
    protected getName(){
        return 'task_candidate'
    }
}
