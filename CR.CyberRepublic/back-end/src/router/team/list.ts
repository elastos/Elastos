import Base from '../Base'
import TeamService from '../../service/TeamService'
import * as _ from 'lodash'
import {constant} from '../../constant'

export default class extends Base{

    /**
     * For consistency we call the service
     * with the entire query
     *
     * @param param
     * @returns {Promise<["mongoose".Document]>}
     */
    public async action(){
        const teamService = this.buildService(TeamService)
        const param = this.getParam()

        if (param.search) {
            param.name = { $regex: _.trim(param.search), $options: 'i' }
        }

        const list = await teamService.list(_.omit(param, ['search']))
        const count = await teamService.getDBModel('Team')
            .count(_.omit(param, ['search', 'page', 'results', 'sortBy', 'sortOrder']))

        return this.result(1, {
            list,
            total: count
        })
    }
}
