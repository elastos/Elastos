import BaseService from '../model/BaseService'
import { api_request } from '@/util'

export default class extends BaseService {
  constructor() {
    super()
    this.selfRedux = this.store.getRedux('cvote')
    this.prefixPath = '/api/cvote'
  }

  async getData(param) {
    const path = `${this.prefixPath}/get/${param.id}`
    this.dispatch(this.selfRedux.actions.loading_update(true))

    const rs = await api_request({
      path,
      method: 'get'
      // data: param,
    })

    this.dispatch(this.selfRedux.actions.data_update(rs))

    return rs
  }

  async resetData() {
    this.dispatch(this.selfRedux.actions.data_update(undefined))
  }

  async createDraft(param) {
    const path = `${this.prefixPath}/create_draft`

    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }

  async updateDraft(param) {
    const path = `${this.prefixPath}/update_draft`

    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }

  async deleteDraft(param) {
    const path = `${this.prefixPath}/delete_draft`
    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }

  async createCVote(param) {
    const path = `${this.prefixPath}/create`

    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }

  async updateCVote(param) {
    const path = `${this.prefixPath}/update`

    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }

  async vote(param) {
    const path = `${this.prefixPath}/vote`

    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }

  async finishCVote(param) {
    const path = `${this.prefixPath}/finish`

    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }

  async unfinishCVote(param) {
    const path = `${this.prefixPath}/unfinish`

    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }

  async updateNotes(param) {
    const path = `${this.prefixPath}/update_notes`

    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }

  async listData(param, isCouncil) {
    let result

    if (isCouncil) {
      result = await api_request({
        path: `${this.prefixPath}/list`,
        method: 'get',
        data: param
      })
    } else {
      result = await api_request({
        path: `${this.prefixPath}/list_public`,
        method: 'get',
        data: param
      })
    }

    return result
  }
}
