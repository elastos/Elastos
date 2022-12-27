import Base from '../Base'
import * as _ from 'lodash'
import { Types } from 'mongoose'
import Service from '../../service/PermissionRoleService'

const ObjectId = Types.ObjectId

const FILTERS = {
  ALL: 'all',
  CREATED: 'createdBy',
  COMMENTED: 'commented',
  SUBSCRIBED: 'subscribed'
}

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
    const service = this.buildService(Service)
    const param = this.getParam()
    const result = await service.list(param)

    return this.result(1, result)
  }
}
