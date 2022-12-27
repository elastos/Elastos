import Base from './Base'

export default class extends Base {
    protected getSchema(){
        return {
            name : String,
            age : Number,
            time : Date
        }
    }
    protected getName(){
        return 'Test'
    }

}