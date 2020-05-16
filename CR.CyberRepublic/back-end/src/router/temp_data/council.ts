import Base from '../Base'
import TempDataService from '../../service/TempDataService'

export default class extends Base {
    protected needLogin = false

    public async action() {
        const tempDataService = this.buildService(TempDataService)

        const rs = await tempDataService.listCouncil()

        return this.result(1, rs)
    }
}
