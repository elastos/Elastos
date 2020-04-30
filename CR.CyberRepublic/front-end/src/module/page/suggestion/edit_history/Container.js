import _ from 'lodash'
import {
  createContainer,
} from '@/util'
import Component from './Component'
import SuggestionService from '@/service/SuggestionService'

const mapState = state => ({
  dataList: state.suggestion.edit_history,
  loading: _.get(state.suggestion.detail, 'loading'),
  detail: state.suggestion.detail,
  user: state.user,
  currentUserId: state.user.current_user_id,
  isCouncil: state.user.is_council,
  isAdmin: state.user.is_admin,
})

const mapDispatch = () => {
  const service = new SuggestionService()

  return {
    getEditHistories({ id }) {
      return service.editHistories({ id })
    },
    resetEditHistory() {
      return service.resetEditHistory()
    },
    revertVersion(id, version) {
      return service.revertVersion(id, version)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
