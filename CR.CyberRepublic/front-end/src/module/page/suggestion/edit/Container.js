import {
  createContainer,
} from '@/util'
import Component from './Component'
import SuggestionService from '@/service/SuggestionService'

export default createContainer(Component, (state) => {
  return {
    currentUserId: state.user.current_user_id,
    isCouncil: state.user.is_council,
    isAdmin: state.user.is_admin,
    detail: state.suggestion.detail,
    draft: state.suggestion.draft,
  }
}, () => {
  const service = new SuggestionService()

  return {
    getDetail(id) {
      return service.getDetail({
        id,
        incViewsNum: false,
      })
    },
    updateSuggestion(suggestion) {
      return service.update(suggestion)
    },
    getDraft(id) {
      return service.getDraft(id)
    },
    saveDraft(suggestion) {
      return service.saveDraft(suggestion)
    }
  }
})
