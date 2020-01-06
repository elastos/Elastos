import _ from 'lodash'
import {
  createContainer,
} from '@/util'
import {
  SUGGESTION_STATUS,
  SUGGESTION_SEARCH_FILTERS
} from '@/constant'
import SuggestionService from '@/service/SuggestionService'
import CommentService from '@/service/CommentService'
import Component from './Component'

const excludeFilters = (value, key) => !['search', 'filter'].includes(key)

const defaultFilters = {
  referenceStatus: false,
  infoNeeded: false,
  underConsideration: false,
  search: '',
  filter: SUGGESTION_SEARCH_FILTERS.TITLE,
  status: SUGGESTION_STATUS.ACTIVE,
  budgetRequested: '',
  creationDate: [],
  author: '',
  type: ''
}

const mapState = (state) => {
  const currentUserId = state.user.current_user_id

  const suggestionState = {
    ...state.suggestion,
    tagsIncluded: state.suggestion.tags_included,
    referenceStatus: state.suggestion.reference_status,
    dataList: state.suggestion.all_suggestions,
    total: state.suggestion.all_suggestions_total,
    currentUserId,
    filters: _.isEmpty(state.suggestion.filters)
      ? defaultFilters
      : state.suggestion.filters,
    isVisitableFilter: _.isEmpty(state.suggestion.filters)
      ? false
      : !_.isEqual(
        _.filter(defaultFilters, excludeFilters),
        _.filter(state.suggestion.filters, excludeFilters)
      ),
    isLogin: state.user.is_login,
    isSecretary: state.user.is_secretary,
    user: state.user
  }

  return suggestionState
}

const mapDispatch = () => {
  const service = new SuggestionService()
  const commentService = new CommentService()

  return {
    getDefaultFilters() {
      return defaultFilters
    },
    async updateFilters(filters) {
      return service.updateFilters({...defaultFilters, ...filters})
    },

    async clearFilters() {
      return this.updateFilters({})
    },

    async changePage(page) {
      return service.changePage(page)
    },

    async onSortByChanged(sortBy) {
      return service.saveSortBy(sortBy)
    },

    async onTagsIncludedChanged(tagsIncluded) {
      return service.saveTagsIncluded(tagsIncluded)
    },

    async onReferenceStatusChanged(referenceStatus) {
      return service.saveReferenceStatus(referenceStatus)
    },

    async getList(query) {

      // query = Object.assign({
      //   status: SUGGESTION_STATUS.ACTIVE
      // }, query)

      return service.list(query)
    },

    async loadMore(query) {

      // query = Object.assign({
      //   status: SUGGESTION_STATUS.ACTIVE
      // }, query)

      return service.loadMore(query)
    },

    resetAll() {
      return service.resetAll()
    },

    async create(doc) {
      return service.create(doc)
    },

    async reportAbuse(id) {
      return service.reportAbuse(id)
    },
    async subscribe(id) {
      return commentService.subscribe('suggestion', id)
    },

    async unsubscribe(id) {
      return commentService.unsubscribe('suggestion', id)
    },

    async exportAsCSV(query) {
      return service.exportAsCSV(query)
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
