import Base from '../Base'
import CommunityService from '../../service/CommunityService'
import {constant} from '../../constant'

export default class extends Base {
    protected needLogin = false

    async action(){
        const communityService = this.buildService(CommunityService)
        const param = this.getParam()
        const rs = await communityService.index(param)
        return this.result(1, rs)
    }

}
