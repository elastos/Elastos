import _ from 'lodash'
import { createContainer } from '@/util'
import Component from './Component'
import ElipService from '@/service/ElipService'

const excludeFilters = (value, key) => (key !== 'search')

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
  filters: _.isEmpty(state.elip.filters) ? defaultFilters : state.elip.filters,
  isVisitableFilter: _.isEmpty(state.elip.filters)
    ? false
    : !_.isEqual(
      _.filter(defaultFilters, excludeFilters),
      _.filter(state.elip.filters, excludeFilters)
    ),
  sortBy: state.elip.sortBy
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
    },
    async onSortByChanged(sortBy) {
      return service.saveSortBy(sortBy)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
