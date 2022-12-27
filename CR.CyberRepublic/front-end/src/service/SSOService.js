import BaseService from '../model/BaseService'
import { api_request } from '@/util'

export default class extends BaseService {
  async getLoginStr(params = {}) {
    const result = await api_request({
      path: '/api/sso/login',
      method: 'get',
      data: params,
    })

    return result
  }
}
