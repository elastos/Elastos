import Base from '../Base'
import TeamService from '../../service/TeamService'

export default class extends Base {
    protected needLogin = true
    async action(){
        const param = this.getParam()
        const {action} = param

        const teamService = this.buildService(TeamService)

        let rs = undefined
        if (action === 'accept') {
            rs = await teamService.acceptApply(param)
        } else if (action === 'reject') {
            rs = await teamService.rejectApply(param)
        } else if (action === 'withdraw') {
            rs = await teamService.withdrawApply(param)
        } else if (action === 'delete') {
            rs = await teamService.deleteTeam(param)
        } else {
            return this.res.sendStatus(403)
        }

        return this.result(1, rs)
    }
}
