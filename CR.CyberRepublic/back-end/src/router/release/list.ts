import Base from '../Base'
import ReleaseService from '../../service/ReleaseService'

export default class extends Base {
  protected needLogin = false

  /**
   * For consistency we call the service
   * with the entire query
   *
   * @param param
   * @returns {Promise<["mongoose".Document]>}
   */
  public async action() {
    const service = this.buildService(ReleaseService)
    const result = await service.list()

    return this.result(1, result)
  }
}
