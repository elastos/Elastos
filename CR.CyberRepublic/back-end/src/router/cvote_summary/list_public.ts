import * as _ from 'lodash'
import Base from '../Base'
import CVoteSummaryService from '../../service/CVoteSummaryService'

export default class extends Base {

  // protected needLogin = true;

  async action() {
    const param = this.getParam()
    const service = this.buildService(CVoteSummaryService)

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
