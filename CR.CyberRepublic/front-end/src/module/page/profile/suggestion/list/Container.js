import _ from 'lodash'
import { createContainer } from '@/util'
import { SUGGESTION_STATUS } from '@/constant'
import SuggestionService from '@/service/SuggestionService'
import Component from './Component'

const mapState = (state) => {
  const currentUserId = state.user.current_user_id
  // const isAdmin = state.user.role === USER_ROLE.ADMIN

  const suggestionState = {
    ...state.suggestion,
    dataList: state.suggestion.my_suggestions,
    total: state.suggestion.my_suggestions_total,
    currentUserId,
    filter: state.suggestion.filter || {},
  }

  return suggestionState
}

const mapDispatch = () => {
  const service = new SuggestionService()

  return {
    async getList(query) {
      return service.myList({
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
  }
}

export default createContainer(Component, mapState, mapDispatch)
