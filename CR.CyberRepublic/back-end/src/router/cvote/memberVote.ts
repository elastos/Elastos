import Base from '../Base'
import CVoteService from '../../service/CVoteService'

export default class extends Base{
    protected needLogin = true;
    async action(){
        const param = this.getParam()
        const service = this.buildService(CVoteService)

        const rs = await service.memberVote(param)
        return this.result(1, rs)
    }
}
