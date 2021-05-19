import { createContainer } from '@/util'
import Component from './Component'
import CVoteTrackingService from '@/service/CVoteTrackingService'

const mapState = state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary,
  isCouncil: state.user.is_council,
  canManage: state.user.is_secretary || state.user.is_council,
})

const mapDispatch = () => {
  const service = new CVoteTrackingService()
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
