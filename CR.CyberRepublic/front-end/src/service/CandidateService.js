import BaseService from '../model/BaseService'
import { wallet_request } from '@/util'

export default class extends BaseService {
  constructor() {
    super()
    this.selfRedux = this.store.getRedux('candidate')
    this.prefixPath = '/api/dposnoderpc/check'
  }

  async listData(param) {
    const path = `${this.prefixPath}/listcrcandidates`
    const rs = await wallet_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }
}
