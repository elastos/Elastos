import Base from '../Base'
import CVoteService from '../../service/CVoteService'

export default class extends Base {
    protected needLogin = false

    async action(){
        const param = this.getParam()
        const service = this.buildService(CVoteService)

        const id = param.id
        const rs = await service.getProposalById(id)
        return this.result(1, rs)
    }
}
