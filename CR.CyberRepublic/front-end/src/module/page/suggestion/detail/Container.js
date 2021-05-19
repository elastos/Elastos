import { createContainer } from '@/util'
import _ from 'lodash'
import Component from './Component'
import SuggestionService from '@/service/SuggestionService'
import CommentService from '@/service/CommentService'
import CVoteService from '@/service/CVoteService'

export default createContainer(
  Component,
  (state) => {
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
      isAdmin: state.user.is_admin,
      isReference: !_.isEmpty(
        state.suggestion.detail && state.suggestion.detail.reference
      ) || state.suggestion.reference_status
    }
  },
  () => {
    const service = new SuggestionService()
    const commentService = new CommentService()
    const cVoteService = new CVoteService()
    return {
      async getDetail({ id, incViewsNum }) {
        return service.getDetail({
          id,
          incViewsNum
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
      async needDueDiligence(id) {
        return service.needDueDiligence(id)
      },
      async needAdvisory(id) {
        return service.needAdvisory(id)
      },
      async getDraft(id) {
        return service.getDraft(id)
      },
      async getSignatureUrl(id) {
        return service.getSignatureUrl(id)
      },
      async getSignature(id) {
        return service.getSignature(id)
      },
      async getCMSignatureUrl(id) {
        return service.getCMSignatureUrl(id)
      }
    }
  }
)
