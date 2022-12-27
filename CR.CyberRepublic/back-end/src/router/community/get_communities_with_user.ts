import Base from '../Base'
import CommunityService from '../../service/CommunityService'
import UserService from '../../service/UserService'
import {constant} from '../../constant'
import * as _ from 'lodash'

export default class extends Base {
    protected needLogin = false
    async action(){

        const userId = this.getParam('userId')
        return await this.show(userId)
    }

    async show(userId) {
        const communityService = this.buildService(CommunityService)
        const userService = this.buildService(UserService)

        const rs = await communityService.listCommunity({userId})
        const communityIds = this.getCommunityIds(rs)
        const communities = await communityService.findCommunities({communityIds})

        return this.result(1, communities)
    }

    getCommunityIds(items) {
        return _.map(items, 'communityId')
    }
}
