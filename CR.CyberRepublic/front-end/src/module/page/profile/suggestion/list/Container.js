import _ from 'lodash'
import { createContainer } from '@/util'
import { SUGGESTION_STATUS } from '@/constant'
import SuggestionService from '@/service/SuggestionService'
import Component from './Component'

const mapState = (state) => {
  const currentUserId = state.user.current_user_id
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
      return service.myList(query)
    },

    resetAll() {
      return service.resetMyList()
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
