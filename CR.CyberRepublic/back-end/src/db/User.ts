import Base from './Base'
import {User} from './schema/UserSchema'

/**
 * We never return password and salt, please note this is only provided
 * using the DB/Base helper, if you are using getInstance for populate, you need to use selectFields
 *
 * TODO: I believe from a standpoint this design is wrong, we should not partially extend the base Mongoose class,
 * we can however provide a helper class/DAO but masking the base Mongoose is not good design
 */
export default class extends Base {
    protected getSchema(){
        return User
    }
    protected getName(){
        return 'users'
    }
    protected rejectFields(){
        return {
            password : false,
            salt : false
        }
    }
}
