import {
  createContainer,
} from '@/util'
import Component from './Component'
import SuggestionService from '@/service/SuggestionService'

export default createContainer(Component, (state) => {
  return {
    currentUserId: state.user.current_user_id,
    isCouncil: state.user.is_council,
    isAdmin: state.user.is_admin
  }
}, () => {
  const service = new SuggestionService()

  return {
    createSuggestion(suggestion) {
      return service.create(suggestion)
    }
  }
})
