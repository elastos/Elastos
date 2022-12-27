import Base from '../Base'
import CVoteService from '../../service/CVoteService'

export default class extends Base{
    async action(){
        const service = this.buildService(CVoteService)

        const rs = await service.temporaryChangeUpdateStatus()
        return this.result(1, rs)
    }
}
