import Base from './Base'
import { CVote_Tracking } from './schema/CVoteTrackingSchema'

export default class extends Base {
    protected getSchema(){
        return CVote_Tracking
    }
    protected getName(){
        return 'cvote_tracking'
    }
    protected rejectFields(){
        return {}
    }
}
