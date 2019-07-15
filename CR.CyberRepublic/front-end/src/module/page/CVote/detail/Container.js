import { createContainer } from '@/util'
import Component from './Component'
import _ from 'lodash'
import { avatar_map } from '@/constant'
import CVoteService from '@/service/CVoteService'

const mapState = state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary,
  isCouncil: state.user.is_council,
  canManage: state.user.is_secretary || state.user.is_council,
  avatar_map,
  trackingStatus: _.get(_.last([...state.cvoteTracking.all_public, ...state.cvoteTracking.all_private]), 'status'),
  summaryStatus: _.get(_.last([...state.cvoteSummary.all_public, ...state.cvoteSummary.all_private]), 'status'),
})

const mapDispatch = () => {
  const service = new CVoteService()
  return {
    async createDraft(param) {
      return service.createDraft(param)
    },
    async listData(param, isCouncil) {
      return service.listData(param, isCouncil)
    },
    async getData(param) {
      return service.getData(param)
    },
    async createCVote(param) {
      return service.createCVote(param)
    },
    async updateCVote(param) {
      return service.updateCVote(param)
    },
    async vote(param) {
      return service.vote(param)
    },
    async finishCVote(param) {
      return service.finishCVote(param)
    },
    async unfinishCVote(param) {
      return service.unfinishCVote(param)
    },
    async updateNotes(param) {
      return service.updateNotes(param)
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
