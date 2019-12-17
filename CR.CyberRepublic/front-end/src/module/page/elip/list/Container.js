import _ from 'lodash'
import { createContainer } from '@/util'
import Component from './Component'
import ElipService from '@/service/ElipService'

const defaultFilters = {
  search: '',
  filter: '',
  creationDate: [],
  author: '',
  type: ''
}

const mapState = state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary,
  filters: _.isEmpty(state.elip.filters)
    ? defaultFilters
    : state.elip.filters,
  isVisitableFilter: !_.isEqual(defaultFilters, state.suggestion.filters),
})

const mapDispatch = () => {
  const service = new ElipService()
  return {
    getDefaultFilters() {
      return defaultFilters
    },
    async updateFilters(filters) {
      return service.updateFilters({...defaultFilters, ...filters})
    },
    async clearFilters() {
      return this.updateFilters({})
    },
    async listData(param) {
      return service.listData(param)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
