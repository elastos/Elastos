import _ from 'lodash'
import { api_request } from '@/util'
import { logger } from '@/util'
import I18N from '@/I18N'
import { message } from 'antd'
import BaseService from '../model/BaseService'

export default class extends BaseService {
  constructor() {
    super()
    this.selfRedux = this.store.getRedux('suggestion')
    this.prefixPath = '/api/suggestion'
  }

  async changePage(page) {
    this.dispatch(this.selfRedux.actions.page_update(page))
  }

  async saveFilter(filter) {
    this.dispatch(this.selfRedux.actions.filter_update(filter))
  }

  async saveSortBy(sortBy) {
    this.dispatch(this.selfRedux.actions.sortBy_update(sortBy))
  }

  async saveTagsIncluded(tagsIncluded) {
    this.dispatch(this.selfRedux.actions.tags_included_update(tagsIncluded))
  }

  async saveReferenceStatus(referenceStatus) {
    this.dispatch(
      this.selfRedux.actions.reference_status_update(referenceStatus)
    )
  }

  async saveEditHistory(editHistory) {
    this.dispatch(this.selfRedux.actions.edit_history_update(editHistory))
  }

  async loadMore(qry) {
    this.list(qry)
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
        signal: this.getAbortSignal(path)
      })

      this.dispatch(this.selfRedux.actions.loading_update(false))
      this.dispatch(this.selfRedux.actions.all_suggestions_reset())
      this.dispatch(
        this.selfRedux.actions.all_suggestions_total_update(result.total)
      )
      this.dispatch(
        this.selfRedux.actions.all_suggestions_update(_.values(result.list))
      )
    } catch (e) {
      logger.error(e)
    }

    return result
  }

  async myList(qry) {
    this.dispatch(this.selfRedux.actions.my_suggestions_loading_update(true))

    const path = `${this.prefixPath}/list`
    // this.abortFetch(path)

    let result
    try {
      result = await api_request({
        path,
        method: 'get',
        data: qry,
        signal: this.getAbortSignal(path)
      })

      this.dispatch(this.selfRedux.actions.my_suggestions_loading_update(false))
      this.dispatch(this.selfRedux.actions.my_suggestions_reset())
      this.dispatch(
        this.selfRedux.actions.my_suggestions_total_update(result.total)
      )
      this.dispatch(
        this.selfRedux.actions.my_suggestions_update(_.values(result.list))
      )
    } catch (e) {
      // Do nothing
    }

    return result
  }

  resetAll() {
    this.dispatch(this.selfRedux.actions.all_suggestions_reset())
  }

  resetMyList() {
    this.dispatch(this.selfRedux.actions.my_suggestions_reset())
  }

  resetDetail() {
    this.dispatch(this.selfRedux.actions.detail_reset())
    this.dispatch(this.selfRedux.actions.reference_status_reset())
  }

  async create(doc) {
    this.dispatch(this.selfRedux.actions.loading_update(true))

    const path = `${this.prefixPath}/create`
    let res
    try {
      res = await api_request({
        path,
        method: 'post',
        data: doc
      })
    } catch (error) {
      this.dispatch(this.selfRedux.actions.loading_update(false))
      message.error('Error happened, please try again later or contact admin.')
      logger.error(error)
    }
    return res
  }

  async update(doc) {
    this.dispatch(this.selfRedux.actions.loading_update(true))

    const path = `${this.prefixPath}/${doc.id}/update`
    let res
    try {
      res = await api_request({
        path,
        method: 'put',
        data: doc
      })
    } catch (error) {
      this.dispatch(this.selfRedux.actions.loading_update(false))
      message.error('Error happened, please try again later or contact admin.')
      logger.error(error)
    }
    return res
  }

  async addTag(doc) {
    this.dispatch(this.selfRedux.actions.loading_update(true))

    const path = `${this.prefixPath}/${doc.id}/addtag`
    let res
    try {
      res = await api_request({
        path,
        method: 'post',
        data: doc
      })
    } catch (error) {
      this.dispatch(this.selfRedux.actions.loading_update(false))
      message.error('Error happened, please try again later or contact admin.')
      logger.error(error)
    }
    return res
  }

  async getDetail({ id, incViewsNum }) {
    this.dispatch(this.selfRedux.actions.loading_update(true))

    const path = `${this.prefixPath}/${id}/show`
    const result = await api_request({
      path,
      method: 'get',
      data: { incViewsNum }
    })

    this.dispatch(this.selfRedux.actions.loading_update(false))
    this.dispatch(this.selfRedux.actions.detail_update(result))

    return result
  }

  async editHistories({ id }) {
    this.dispatch(this.selfRedux.actions.loading_update(true))
    const path = `${this.prefixPath}/${id}/editHistories`
    const res = await api_request({
      path,
      method: 'get'
    })
    const dpath = `${this.prefixPath}/${id}/show`
    const result = await api_request({
      path: dpath,
      method: 'get'
    })

    this.dispatch(this.selfRedux.actions.detail_update(result))
    this.dispatch(this.selfRedux.actions.edit_history_update(res))

    this.dispatch(this.selfRedux.actions.loading_update(false))
  }

  async revertVersion(id, version) {
    this.dispatch(this.selfRedux.actions.loading_update(true))

    const path = `${this.prefixPath}/${id}/revertVersion`
    let res
    try {
      res = await api_request({
        path,
        method: 'post',
        data: { version }
      })

      if (res.version) {
        this.editHistories({ id })
        message.info(I18N.get('suggestion.msg.revertVersion'))
      }
    } catch (error) {
      this.dispatch(this.selfRedux.actions.loading_update(false))
      message.error('Error happened, please try again later or contact admin.')
      logger.error(error)
    }
  }

  resetEditHistory() {
    this.dispatch(this.selfRedux.actions.edit_history_reset())
  }

  async like(id) {
    const path = `${this.prefixPath}/${id}/like`

    const res = await api_request({
      path,
      method: 'post'
    })

    return res
  }

  async dislike(id) {
    const path = `${this.prefixPath}/${id}/dislike`

    const res = await api_request({
      path,
      method: 'post'
    })

    return res
  }

  async reportAbuse(id) {
    const path = `${this.prefixPath}/${id}/reportabuse`

    const res = await api_request({
      path,
      method: 'post'
    })

    return res
  }

  async abuse(id) {
    const path = `${this.prefixPath}/${id}/abuse`

    const res = await api_request({
      path,
      method: 'post'
    })

    return res
  }

  // ADMIN and Author
  async archiveOrUnarchive(data) {
    const path = `${this.prefixPath}/${data.id}/archive`
    const res = await api_request({
      path,
      data,
      method: 'post'
    })
    if (res.success && !data.isArchived) {
      message.info(I18N.get('suggestion.msg.archived'))
    }
    if (res.success && data.isArchived === true) {
      message.info(I18N.get('suggestion.msg.unarchived'))
    }
    return res
  }

  // ADMIN ONLY
  async delete(id) {
    const path = `${this.prefixPath}/${id}/delete`

    const res = await api_request({
      path,
      method: 'post'
    })

    return res
  }

  // ADMIN and Council
  async needDueDiligence(id) {
    const path = `${this.prefixPath}/${id}/investigation`
    const res = await api_request({
      path,
      method: 'post'
    })
    return res
  }

  // ADMIN and Council
  async needAdvisory(id) {
    const path = `${this.prefixPath}/${id}/advisory`
    const res = await api_request({
      path,
      method: 'post'
    })
    return res
  }

  async exportAsCSV(qry) {
    const path = `${this.prefixPath}/export2csv`
    this.abortFetch(path)

    let result
    try {
      result = await api_request({
        path,
        method: 'get',
        data: qry,
        headers: {
          'Content-Type': 'text/csv'
        }
      })
    } catch (e) {
      logger.error(e)
    }

    return result
  }

  async updateFilters(filters) {
    this.dispatch(this.selfRedux.actions.filters_update(filters))
  }

  async saveDraft(doc) {
    this.dispatch(this.selfRedux.actions.loading_update(true))

    const path = `${this.prefixPath}/${doc.id}/saveDraft`
    let res
    try {
      res = await api_request({
        path,
        method: 'put',
        data: doc
      })
    } catch (error) {
      this.dispatch(this.selfRedux.actions.loading_update(false))
      message.error('Error happened, please try again later or contact admin.')
      logger.error(error)
    }
    return res
  }

  async getDraft(id) {
    this.dispatch(this.selfRedux.actions.loading_update(true))
    this.dispatch(this.selfRedux.actions.draft_reset())

    const path = `${this.prefixPath}/${id}/showDraft`
    const result = await api_request({
      path,
      method: 'get'
    })

    this.dispatch(this.selfRedux.actions.loading_update(false))
    this.dispatch(this.selfRedux.actions.draft_update(result))
    return result
  }

  // signature
  async getSignatureUrl(id) {
    const path = `${this.prefixPath}/signature-url`
    const res = await api_request({
      path,
      method: 'post',
      data: { id }
    })
    return res
  }

  async getSignature(id) {
    const path = `${this.prefixPath}/signature`
    const rs = await api_request({
      path,
      method: 'post',
      data: { id }
    })
    if (rs && rs.success && rs.data) {
      this.dispatch(this.selfRedux.actions.detail_update(rs.data))
    }
    return rs
  }

  async getCMSignatureUrl(id) {
    const path = `${this.prefixPath}/cm-signature-url`
    const res = await api_request({
      path,
      method: 'post',
      data: { id }
    })
    return res
  }
}
