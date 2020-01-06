import _ from 'lodash'
import { createContainer } from '@/util'
import Component from './Component'
import CVoteService from '@/service/CVoteService'

const excludeFilters = (value, key) => (key !== 'search')

const defaultFilters = {
  voteResult: 'all',
  search: '',
  status: '',
  budgetRequested: '',
  hasTrackingMsg: false,
  isUnvotedByYou: false,
  creationDate: [],
  author: '',
  type: '',
  endsDate: [],
}

const mapState = state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary,
  isCouncil: state.user.is_council,
  canManage: state.user.is_secretary || state.user.is_council,
  filters: _.isEmpty(state.cvote.filters)
    ? defaultFilters
    : state.cvote.filters,
  isVisitableFilter: _.isEmpty(state.cvote.filters)
    ? false
    : !_.isEqual(
      _.filter(defaultFilters, excludeFilters),
      _.filter(state.cvote.filters, excludeFilters)
    )
})

const mapDispatch = () => {
  const service = new CVoteService()
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
    async listData(param, isCouncil) {
      return service.listData(param, isCouncil)
    },
    async createDraft(param) {
      return service.createDraft(param)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
