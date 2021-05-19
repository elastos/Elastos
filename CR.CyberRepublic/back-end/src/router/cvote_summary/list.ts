import * as _ from 'lodash'
import Base from '../Base'
import CVoteSummaryService from '../../service/CVoteSummaryService'

export default class extends Base {
  /**
   * @returns {Promise<{code: number; data: any; message: string} | {code: number; type: string; error: string}>}
   */
  async action() {
    const param = this.getParam()
    const service = this.buildService(CVoteSummaryService)
    const rs = await service.list(param)
    return this.result(1, rs)
  }
}
