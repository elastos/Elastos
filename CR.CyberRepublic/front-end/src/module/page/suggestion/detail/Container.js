import {
  createContainer,
} from '@/util'
import Component from './Component'
import SuggestionService from '@/service/SuggestionService'
import CommentService from '@/service/CommentService'
import CVoteService from '@/service/CVoteService'

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
    user: state.user,
    currentUserId: state.user.current_user_id,
    isCouncil: state.user.is_council,
    isAdmin: state.user.is_admin
  }
}, () => {
  const service = new SuggestionService()
  const commentService = new CommentService()
  const cVoteService = new CVoteService()

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
    async createDraft(param) {
      return cVoteService.createDraft(param)
    },
    async update(param) {
      return service.update(param)
    },
    async addTag(param) {
      return service.addTag(param)
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
