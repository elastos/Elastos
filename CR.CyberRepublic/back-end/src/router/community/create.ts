import Base from '../Base'
import CommunityService from '../../service/CommunityService'

export default class extends Base{
    protected needLogin = true
    public async action(){
        const communityService = this.buildService(CommunityService)
        const rs = await communityService.create(this.getParam())

        return this.result(1, rs)
    }
}
