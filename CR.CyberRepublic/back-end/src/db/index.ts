import DB from './DB'

let db: DB

/**
 * Ensure we initialize the db session only once,
 * subsequent requires return the existing session
 */
export default class {
    static async create(){
        if(!db){
            db = new DB()
            await db.start()
        }
        return db
    }

}
