import BaseService from '../model/BaseService'
import { api_request } from '@/util'

export default class extends BaseService {
  async list(filter = {}) {
    const result = await api_request({
      path: '/api/submission/list',
      method: 'get',
      data: filter,
    })

    return result
  }

  async index(qry) {
    const submissionRedux = this.store.getRedux('submission')

    this.dispatch(submissionRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/submission/list',
      method: 'get',
      data: qry,
    })

    this.dispatch(submissionRedux.actions.all_submissions_reset())
    this.dispatch(submissionRedux.actions.all_submissions_update(result.list))

    this.dispatch(submissionRedux.actions.loading_update(false))

    return result
  }

  async get(submissionId) {
    const submissionRedux = this.store.getRedux('submission')

    this.dispatch(submissionRedux.actions.loading_update(true))

    const result = await api_request({
      path: `/api/submission/${submissionId}`,
      method: 'get',
    })

    this.dispatch(submissionRedux.actions.loading_update(false))
    this.dispatch(submissionRedux.actions.detail_update(result))

    return result
  }

  async getExistingSubmission(campaign) {
    await api_request({
      path: `/api/submission/campaign/${campaign}`,
      method: 'get',
    })
  }

  /**
     * @param taskId
     * @param doc
     * @returns {Promise<*>}
     */
  async update(submissionId, doc) {
    const submissionRedux = this.store.getRedux('submission')

    this.dispatch(submissionRedux.actions.loading_update(true))

    const result = await api_request({
      path: `/api/submission/${submissionId}`,
      method: 'put',
      data: doc,
    })

    const curSubmissionDetail = this.store.getState().submission.detail

    this.dispatch(submissionRedux.actions.detail_update(curSubmissionDetail))
    this.dispatch(submissionRedux.actions.loading_update(false))

    return result
  }

  async create(doc) {
    const res = await api_request({
      path: '/api/submission/create',
      method: 'post',
      data: doc,
    })

    return res
  }

  async archive(submissionId) {
    const res = await api_request({
      path: `/api/submission/${submissionId}`,
      method: 'delete',
    })

    return res
  }

  async resetAllSubmissions() {
    const submissionRedux = this.store.getRedux('submission')
    this.dispatch(submissionRedux.actions.all_submissions_reset())
  }

  async resetSubmissionDetail() {
    const submissionRedux = this.store.getRedux('submission')
    this.dispatch(submissionRedux.actions.detail_reset())
  }

  async saveFilter(filter) {
    const submissionRedux = this.store.getRedux('submission')
    this.dispatch(submissionRedux.actions.filter_update(filter))
  }
}
