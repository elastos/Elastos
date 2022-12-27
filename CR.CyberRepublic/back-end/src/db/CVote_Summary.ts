import Base from './Base'
import { CVote_Summary } from './schema/CVoteSummarySchema'

export default class extends Base {
    protected getSchema(){
        return CVote_Summary
    }
    protected getName(){
        return 'cvote_summary'
    }
    protected rejectFields(){
        return {}
    }
}
