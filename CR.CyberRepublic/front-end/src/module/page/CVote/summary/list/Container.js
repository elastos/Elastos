import { createContainer } from '@/util'
import Component from './Component'
import CVoteSummaryService from '@/service/CVoteSummaryService'

const mapState = state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary,
  isCouncil: state.user.is_council,
  canManage: state.user.is_secretary || state.user.is_council,
  publicList: state.cvoteTracking.all_public,
  privateList: state.cvoteTracking.all_private,
})

const mapDispatch = () => {
  const service = new CVoteSummaryService()
  return {
    async create(param) {
      return service.create(param)
    },
    async reject(param) {
      return service.reject(param)
    },
    async approve(param) {
      return service.approve(param)
    },
    async listData(param, isAuthorized = false) {
      return service.listData(param, isAuthorized)
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
