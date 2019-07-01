import BaseService from '../model/BaseService'
import { api_request } from '@/util'

export default class extends BaseService {
  constructor() {
    super()
    // this.selfRedux = this.store.getRedux('cvoteTracking')
    this.prefixPath = '/api/cvoteTracking'
  }

  async create(param) {
    const path = `${this.prefixPath}/create`

    const rs = await api_request({
      path,
      method: 'post',
      data: param,
    })
    return rs
  }

  async update(param) {
    const path = `${this.prefixPath}/update`

    const rs = await api_request({
      path,
      method: 'post',
      data: param,
    })
    return rs
  }

  async approve(param) {
    const path = `${this.prefixPath}/approve`

    const rs = await api_request({
      path,
      method: 'post',
      data: param,
    })
    return rs
  }

  async reject(param) {
    const path = `${this.prefixPath}/reject`

    const rs = await api_request({
      path,
      method: 'post',
      data: param,
    })
    return rs
  }

  async listData(param, isAuthorized) {
    let result

    if (isAuthorized) {
      result = await api_request({
        path: `${this.prefixPath}/list`,
        method: 'get',
        data: param,
      })
    } else {
      result = await api_request({
        path: `${this.prefixPath}/list_public`,
        method: 'get',
        data: param,
      })
    }

    return result
  }

}
