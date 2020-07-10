import Base from '../Base'
import { ela } from '../../utility'
import CVoteService from '../../service/CVoteService'

export default class extends Base {
    // protected needLogin = true;
    async action(){
        const service = this.buildService(CVoteService)
        const rs = await service.getCurrentHeight()
        return this.result(1, rs)
    }
}