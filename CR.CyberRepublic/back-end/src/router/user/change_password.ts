import Base from '../Base'
import UserService from '../../service/UserService'


export default class extends Base {
    protected needLogin = true
    async action(){
        const userService = this.buildService(UserService)
        await userService.changePassword(this.getParam())
        return this.result(1, {})
    }
}
