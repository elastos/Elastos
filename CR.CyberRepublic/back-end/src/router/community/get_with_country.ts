import Base from '../Base'
import CommunityService from '../../service/CommunityService'
import {constant} from '../../constant'

export default class extends Base {
    protected needLogin = false
    async action(){

        const countryName = this.getParam('countryName')

        return await this.show(countryName)
    }

    async show(countryName) {
        const communityService = this.buildService(CommunityService)
        const rs = await communityService.index({
            query: {
                geolocation: countryName
            }
        })

        return this.result(1, rs)
    }
}
