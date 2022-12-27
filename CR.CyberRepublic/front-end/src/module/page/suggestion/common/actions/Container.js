import {
  createContainer,
} from '@/util'
import {
  SUGGESTION_STATUS,
  USER_ROLE
} from '@/constant'
import SuggestionService from '@/service/SuggestionService'
import CommentService from '@/service/CommentService'
import Component from './Component'

const mapState = (state) => {
  const currentUserId = state.user.current_user_id

  const suggestionState = {
    ...state.suggestion,
    currentUserId,
    isAdmin: state.user.role === USER_ROLE.ADMIN,
    isCouncil: state.user.role === USER_ROLE.COUNCIL,
    isLogin: state.user.is_login,
  }

  return suggestionState
}

const mapDispatch = () => {
  const service = new SuggestionService()
  const commentService = new CommentService()

  return {
    async getList(query) {
      return service.myList({
        status: SUGGESTION_STATUS.ACTIVE,
        ...query,
      })
    },
    async like(id) {
      return service.like(id)
    },
    async dislike(id) {
      return service.dislike(id)
    },
    async reportAbuse(id) {
      return service.reportAbuse(id)
    },
    async archiveOrUnarchive(data) {
      return service.archiveOrUnarchive(data)
    },
    async subscribe(id) {
      return commentService.subscribeWithoutRedux('suggestion', id)
    },

    async unsubscribe(id) {
      return commentService.unsubscribeWithoutRedux('suggestion', id)
    },
    resetAll() {
      return service.resetAll()
    },
    saveEditHistory(data) {
      return service.saveEditHistory(data)
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
