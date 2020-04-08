import Base from '../Base'
import UserService from '../../service/UserService'
import {utilCrypto} from '../../utility'
import * as moment from 'moment'

export default class extends Base {

    /**
     * The API token only encrypts stores the userId, on a request we always query the full user, nothing else
     * passed from the client is trusted.
     *
     * Since we encrypt with the APP_SECRET on the server side we trust that
     *
     * @returns {Promise<{code: number; data: any; message: string} | {code: number; type: string; error: string}>}
     */
    async action(){

        const userService = this.buildService(UserService)

        const {username, password} = this.getParam()

        userService.validate_username(username)
        userService.validate_password(password)


        // first get user for salt
        const salt = await userService.getUserSalt(username)
        const pwd = userService.getPassword(password, salt)

        const user = await userService.findUser({
            username,
            password : pwd
        })
        if(!user){
            throw 'username or password is incorrect'
        }

        
        const resultData = {
            user
        }

        // record user login date
        userService.recordLogin({ userId: user.id })

        // always return api-token on login, this is needed for future requests
        this.session.userId = user.id
        resultData['api-token'] = utilCrypto.encrypt(JSON.stringify({
            userId : user.id,
            expired : moment().add(30, 'd').unix()
        }))

        return this.result(1, resultData)
    }
}
