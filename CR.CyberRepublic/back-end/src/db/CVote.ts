import Base from './Base';
import {CVote} from './schema/CVoteSchema';

export default class extends Base {
    protected getSchema(){
        return CVote;
    }
    protected getName(){
        return 'cvote'
    }
    protected rejectFields(){
        return {
            
        };
    }
}
