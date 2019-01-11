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

  const suggestionState = {
    ...state.suggestion,
    currentUserId,
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
    async subscribe(id) {
      return commentService.subscribeWithoutRedux('suggestion', id)
    },

    async unsubscribe(id) {
      return commentService.unsubscribeWithoutRedux('suggestion', id)
    },
    resetAll() {
      return service.resetAll()
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
