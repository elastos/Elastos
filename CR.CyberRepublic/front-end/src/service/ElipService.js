import BaseService from '../model/BaseService'
import { api_request } from '@/util'

export default class extends BaseService {
  constructor() {
    super()
    this.selfRedux = this.store.getRedux('elip')
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
    const path = `${this.prefixPath}/update`
    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return rs
  }

  async getData(param) {
    this.dispatch(this.selfRedux.actions.loading_update(true))
    const path = `${this.prefixPath}/detail/${param.id}`
    const rs = await api_request({
      path,
      method: 'get'
    })
    this.dispatch(this.selfRedux.actions.detail_update(rs.elip))
    this.dispatch(this.selfRedux.actions.reviews_update(rs.reviews))
    this.dispatch(this.selfRedux.actions.loading_update(false))
    return rs
  }

  async resetData() {
    this.dispatch(this.selfRedux.actions.detail_reset())
    this.dispatch(this.selfRedux.actions.reviews_reset())
  }

  async listData(param) {
    const path = `${this.prefixPath}/list`
    const rs = await api_request({
      path,
      method: 'get',
      data: param
    })
    return rs
  }

  async deleteData(param) {
    const path = `${this.prefixPath}/${param.id}/remove`
    const rs = await api_request({
      path,
      method: 'get'
    })
    return rs
  }

  async updateFilters(filters) {
    this.dispatch(this.selfRedux.actions.filters_update(filters))
  }

  async saveSortBy(sortBy) {
    this.dispatch(this.selfRedux.actions.sortBy_update(sortBy))
  }
}
