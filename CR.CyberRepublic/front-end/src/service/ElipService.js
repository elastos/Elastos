import BaseService from '../model/BaseService'
import { api_request } from '@/util'

export default class extends BaseService {
  constructor() {
    super()
    this.prefixPath = '/api/elip'
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

  async update(param) {
    return {
      vid: 1,
      title: 'CR100',
      author: 'Fay Li',
      status: 'DRAFT',
      created: '2019-8-22'
    }
  }

  async getData(param) {
    const path = `${this.prefixPath}/detail/${param.id}`
    const rs = await api_request({
      path,
      method: 'get'
    })
    return rs
  }

  async listData(param, isSecretary) {
    let result = []

    for (let i = 0; i < 13; i++) {
      result.push({
        vid: i,
        title: 'additional meeting request',
        author: 'Fay Li',
        status: 'WAIT_FOR_REVIEW',
        created: '2019-8-22'
      })
    }
    return result
  }
}
