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
})

const mapDispatch = () => {
  const service = new CVoteSummaryService()
  return {
    async create(param) {
      return service.create(param)
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
