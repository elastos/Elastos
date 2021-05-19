import Base from './Base'
import {Log} from './schema/LogSchema'
import * as mongoose from 'mongoose'

export default class extends Base {
    protected getSchema(){
        return Log
    }
    protected getName(){
        return 'logs'
    }

    protected getSchemaOption(){
        return {
            strict: false,
            versionKey: false
        }
    }
}
