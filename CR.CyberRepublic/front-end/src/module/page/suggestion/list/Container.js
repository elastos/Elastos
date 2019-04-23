import {
  createContainer,
} from '@/util'
import {
  SUGGESTION_STATUS,
} from '@/constant'
import SuggestionService from '@/service/SuggestionService'
import CommentService from '@/service/CommentService'
import Component from './Component'

const mapState = (state) => {
  const currentUserId = state.user.current_user_id
  // const isAdmin = state.user.role === USER_ROLE.ADMIN

  const suggestionState = {
    ...state.suggestion,
    tagsExcluded: state.suggestion.tags_excluded,
    dataList: state.suggestion.all_suggestions,
    total: state.suggestion.all_suggestions_total,
    currentUserId,
    filter: state.suggestion.filter || {},
    isLogin: state.user.is_login,
  }

  return suggestionState
}

const mapDispatch = () => {
  const service = new SuggestionService()
  const commentService = new CommentService()

  return {
    async onSortByChanged(sortBy) {
      return service.saveSortBy(sortBy)
    },

    async onTagsExcludedChanged(tagsExcluded) {
      return service.saveTagsExcluded(tagsExcluded)
    },

    async getList(query) {
      return service.list({
        status: SUGGESTION_STATUS.ACTIVE,
        ...query,
      })
    },

    async loadMore(query) {
      return service.loadMore({
        status: SUGGESTION_STATUS.ACTIVE,
        ...query,
      })
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
  }
}

export default createContainer(Component, mapState, mapDispatch)
