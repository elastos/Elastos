import BaseService from '../model/BaseService'
import { api_request } from '@/util'

export default class extends BaseService {
  constructor() {
    super()
    this.prefixPath = '/api/elip'
  }

  async create(param) {
    return {
      vid: 1,
      title: 'additional meeting request',
      author: 'Fay Li',
      status: 'DRAFT',
      created: '2019-8-22'
    }
  }

  async listData(param, isSecretary) {
    let result = []

    for(let i = 0; i < 13; i++) {
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
