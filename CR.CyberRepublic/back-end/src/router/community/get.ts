import Base from '../Base'
import CommunityService from '../../service/CommunityService'
import {constant} from '../../constant'

export default class extends Base {
    protected needLogin = false
    async action(){

        const communityId = this.getParam('communityId')

        if (!communityId) {
            // TODO: pass in query params later
            return await this.index()
        }
        return await this.show(communityId)
    }

    async show(communityId) {
        const communityService = this.buildService(CommunityService)
        const rs = await communityService.get(communityId)

        return this.result(1, rs)
    }

    async index() {
        const communityService = this.buildService(CommunityService)
        const rs = await communityService.index({
            query: {
                type: constant.COMMUNITY_TYPE.COUNTRY
            }
        })

        return this.result(1, rs)
    }
}
