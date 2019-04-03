import Base from '../Base'
import TeamService from '../../service/TeamService'

export default class extends Base {
    protected needLogin = true
    async action(){
        const param = this.getParam()
        const teamService = this.buildService(TeamService)

        const rs = await teamService.addCandidate(param)
        return this.result(1, rs)
    }
}