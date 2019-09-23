import * as _ from 'lodash'
import Base from '../Base'
import CVoteTrackingService from '../../service/CVoteTrackingService'

export default class extends Base {
  /**
   * @returns {Promise<{code: number; data: any; message: string} | {code: number; type: string; error: string}>}
   */
  async action() {
    const param = this.getParam()
    const service = this.buildService(CVoteTrackingService)
    const rs = await service.list(param)
    return this.result(1, rs)
  }
}
