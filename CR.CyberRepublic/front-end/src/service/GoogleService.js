import BaseService from '../model/BaseService'
import { api_request } from '@/util'

export default class extends BaseService {
  constructor() {
    super()
    // this.selfRedux = this.store.getRedux('google')
    this.prefixPath = '/api/google'
  }

  async translate(param) {
    const path = `${this.prefixPath}/translate`

    const res = await api_request({
      path,
      method: 'post',
      data: param,
    })

    return res
  }
}
