import * as _ from 'lodash'
import Base from '../Base'
import CouncilService from '../../service/CouncilService'

export default class extends Base {
    async action() {
        const param = this.getParam()
        const service = this.buildService(CouncilService)

        service.cronJob();

        const rs = await service.term(param)
        return this.result(1, rs)
    }
}
