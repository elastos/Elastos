import * as _ from 'lodash'
import Base from '../Base'
import ElipService from '../../service/ElipService'

export default class extends Base {
  async action() {
    const param = this.getParam()
    const service = this.buildService(ElipService)

    if (param.search) {
      param.$or = [
        { title: { $regex: _.trim(param.search), $options: 'i' } },
        { vid: _.toNumber(_.trim(param.search)) || 0 }
      ]
    }

    const rs = await service.list(param)
    return this.result(1, rs)
  }
}
