import {
  createContainer,
} from '@/util'
import Component from './Component'
import SuggestionService from '@/service/SuggestionService'
import CommentService from '@/service/CommentService'

export default createContainer(Component, (state) => {
  let page = 'PUBLIC' // default

  if (/^\/admin/.test(state.router.location.pathname)) {
    page = 'ADMIN'
  } else if (/^\/profile/.test(state.router.location.pathname)) {
    page = 'LEADER'
  }

  return {
    ...state.suggestion,
    page,
    currentUserId: state.user.current_user_id,
  }
}, () => {
  const service = new SuggestionService()
  const commentService = new CommentService()

  return {
    async getDetail({
      id,
      incViewsNum,
    }) {
      return service.getDetail({
        id,
        incViewsNum,
      })
    },

    resetDetail() {
      return service.resetDetail()
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
})
