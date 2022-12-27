import BaseService from '../model/BaseService'
import { api_request } from '@/util'

export default class extends BaseService {
  constructor() {
    super()
    this.prefixPath = '/api/elipReview'
  }

  async create(param) {
    const path = `${this.prefixPath}/create`
    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }
}
