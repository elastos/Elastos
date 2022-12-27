import Base from '../Base'
import UserService from '../../service/UserService'

/**
 * Both the '/' and '/:taskId' routes map to this class
 *
 * TODO: this should have more security, although users are public
 *
 * Do not add "needsLogin"
 *
 * Only if it's a request from an admin do we expose the email
 */
export default class GetUser extends Base {

    async action(){

        const userService = this.buildService(UserService)
        const rs = await userService.show(this.getParam())
        return this.result(1, rs)
    }
}
