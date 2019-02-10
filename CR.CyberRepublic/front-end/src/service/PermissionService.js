import BaseService from '../model/BaseService'
import _ from 'lodash'
import { api_request } from '@/util'

export default class extends BaseService {
  constructor() {
    super()
    this.selfRedux = this.store.getRedux('permission')
    this.prefixPath = '/api/permission'
  }

  async list(qry) {
    this.dispatch(this.selfRedux.actions.loading_update(true))

    const path = `${this.prefixPath}/list`
    this.abortFetch(path)

    let result
    try {
      result = await api_request({
        path,
        method: 'get',
        data: qry,
        signal: this.getAbortSignal(path),
      })

      this.dispatch(this.selfRedux.actions.all_permissions_reset())
      this.dispatch(this.selfRedux.actions.all_permissions_total_update(result.total))
      this.dispatch(this.selfRedux.actions.all_permissions_update(_.values(result.list)))
      this.dispatch(this.selfRedux.actions.loading_update(false))
    } catch (e) {
      // Do nothing
    }

    return result
  }

  resetAll() {
    this.dispatch(this.selfRedux.actions.all_permissions_reset())
  }

  async create(doc) {
    this.dispatch(this.selfRedux.actions.loading_update(true))
    const path = `${this.prefixPath}/create`

    const res = await api_request({
      path,
      method: 'post',
      data: doc,
    })

    this.dispatch(this.selfRedux.actions.loading_update(false))

    return res
  }

  async update(doc) {
    this.dispatch(this.selfRedux.actions.loading_update(true))
    const path = `${this.prefixPath}/update`

    const res = await api_request({
      path,
      method: 'post',
      data: doc,
    })

    this.dispatch(this.selfRedux.actions.loading_update(false))

    return res
  }

  async delete(id) {
    this.dispatch(this.selfRedux.actions.loading_update(true))
    const path = `${this.prefixPath}/${id}/delete`

    const res = await api_request({
      path,
      method: 'post',
    })

    this.dispatch(this.selfRedux.actions.loading_update(false))

    return res
  }
}
