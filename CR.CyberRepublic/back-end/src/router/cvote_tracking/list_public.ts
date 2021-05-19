import * as _ from 'lodash'
import Base from '../Base'
import CVoteTrackingService from '../../service/CVoteTrackingService'

export default class extends Base {

  // protected needLogin = true;

  async action() {
    const param = this.getParam()
    const service = this.buildService(CVoteTrackingService)

    // if (param.search) {
    //     param.$or = [
    //         {title: { $regex: _.trim(param.search), $options: 'i' }},
    //         {vid: _.toNumber(_.trim(param.search)) || 0 },
    //     ]
    // }

    const rs = await service.listPublic(param)
    return this.result(1, rs)
  }
}
