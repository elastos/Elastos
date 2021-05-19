import Base from './Base'
import {Submission} from './schema/SubmissionSchema'

export default class extends Base {
    protected getSchema(){
        return Submission
    }
    protected getName(){
        return 'submission'
    }
}
