import Base from '../Base'
import UserService from '../../service/UserService'

export default class extends Base {
    protected needLogin = false
    async action(){

        const userIds = this.getParam('userIds')

        if (userIds) {
            return await this.show(userIds)
        } else {
            return await this.listUser(this.getParam())
        }
    }

    async show(userIds) {
        const userService = this.buildService(UserService)

        const arrayIds = this.getUserIds(userIds)
        const users = await userService.findUsers({
            userIds : arrayIds
        })

        return this.result(1, users)
    }

    /**
     * TODO: may need to add security
     * @returns {Promise<{code: number; data: any; message: string} | {code: number; type: string; error: string}>}
     */
    async listUser(query) {

        const userService = this.buildService(UserService)
        const users = await userService.findAll(query)

        return this.result(1, users)
    }

    getUserIds(userIds: string) {
        let rs = []
        if(userIds){
            rs = userIds.split(',')
        }
        return rs
    }
}
