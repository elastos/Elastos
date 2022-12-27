import Base from '../Base'
import CommunityService from '../../service/CommunityService'
import {constant} from '../../constant'

export default class extends Base {
    protected needLogin = false
    async action(){

        const communityId = this.getParam('communityId')

        return await this.show(communityId)
    }

    async show(communityId) {
        const communityService = this.buildService(CommunityService)
        const rs = await communityService.index({
            query: {
                parentCommunityId: communityId
            }
        })

        return this.result(1, rs)
    }
}
