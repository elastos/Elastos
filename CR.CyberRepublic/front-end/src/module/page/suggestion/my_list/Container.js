import {
  createContainer,
} from '@/util'
import {
  SUGGESTION_STATUS,
} from '@/constant'
import SuggestionService from '@/service/SuggestionService'
import Component from './Component'

const mapState = (state) => {
  const currentUserId = state.user.current_user_id
  // const isAdmin = state.user.role === USER_ROLE.ADMIN
  const {
    my_suggestions_loading: loading,
    my_suggestions: dataList,
    my_suggestions_total: total,
  } = state.suggestion
  const suggestionState = {
    ...state.suggestion,
    dataList,
    total,
    loading,
    currentUserId,
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
    resetAll() {
      return service.resetMyList()
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
