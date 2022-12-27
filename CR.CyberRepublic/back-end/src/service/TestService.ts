import Base from './Base'
import {Document} from 'mongoose'

export default class extends Base {
    public async getTestList(): Promise<Document[]>{
        const db_test = this.getDBModel('Test')
        return await db_test.find({})
    }
}