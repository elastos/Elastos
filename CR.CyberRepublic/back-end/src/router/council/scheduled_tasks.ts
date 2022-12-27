import Base from '../Base'
import CouncilService from '../../service/CouncilService'
import CVoteService from '../../service/CVoteService'

export default class extends Base {
    async action() {
        // const service = this.buildService(CouncilService)
        const service = this.buildService(CVoteService)

        const rs = await service.updateVoteStatusByChain()
        return this.result(1, rs)
    }
}
