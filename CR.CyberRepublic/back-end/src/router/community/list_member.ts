import Base from '../Base'
import CommunityService from '../../service/CommunityService'
import UserService from '../../service/UserService'
import {constant} from '../../constant'
import * as _ from 'lodash'

export default class extends Base {
    protected needLogin = false
    async action(){

        const communityId = this.getParam('communityId')
        return await this.show(communityId)
    }

    async show(communityId) {
        const communityService = this.buildService(CommunityService)
        const userService = this.buildService(UserService)

        const rs = await communityService.listMember(this.getParam())
        const userIds = this.getUserIds(rs)
        const users = await userService.findUsers({userIds})

        return this.result(1, users)
    }

    getUserIds(items) {
        return _.map(items, 'userId')
    }
}
