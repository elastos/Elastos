import Base from '../Base'
import UserService from '../../service/UserService'

/**
 * Both the '/' and '/:taskId' routes map to this class
 */
export default class GetUser extends Base {

    async action(){

        const userService = this.buildService(UserService)

        // TODO: might want to move this to another service
        const rs = await userService.sendConfirmation(this.getParam())
        return this.result(1, rs)
    }
}
