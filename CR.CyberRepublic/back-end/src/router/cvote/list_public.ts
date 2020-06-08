import * as _ from 'lodash'
import Base from '../Base'
import CVoteService from '../../service/CVoteService'

export default class extends Base {
  // protected needLogin = true;

  async action() {
    const param = this.getParam()
    const service = this.buildService(CVoteService)

    // TODO: this is curious, this should be run using agenda or something
    // service.cronJob()

    if (param.search) {
      param.$or = [
        { title: { $regex: _.trim(param.search), $options: 'i' } },
        { vid: _.toNumber(_.trim(param.search)) || 0 }
      ]
    }

    // for the public call param.published should always be true
    param.published = true

    const rs = await service.list(param)
    return this.result(1, rs)
  }
}
