import * as _ from 'lodash'
import Base from '../Base'
import CVoteService from '../../service/CVoteService'

export default class extends Base {
  protected needLogin = false

  /**
   * This is the arbitrary call, however you need to be logged in to call it
   * further more if attempting to view unpublished proposals without admin access the service will
   * reject it
   *
   * @returns {Promise<{code: number; data: any; message: string} | {code: number; type: string; error: string}>}
   */
  async action() {
    const param = this.getParam()
    const service = this.buildService(CVoteService)

    const rs = await service.allOrSearch(param)
    return this.result(1, rs)
  }
}
