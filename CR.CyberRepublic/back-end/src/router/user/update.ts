import Base from '../Base'
import UserService from '../../service/UserService'

export default class extends Base {
    async action(){
        const param = this.getParam()
        const userService = this.buildService(UserService)

        const rs = await userService.update(param)
        return this.result(1, rs)
    }
}
