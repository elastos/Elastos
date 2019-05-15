import {
  createContainer,
} from '@/util'
import ReleaseService from '@/service/ReleaseService'
import Component from './Component'

const mapState = (state) => {
  const currentUserId = state.user.current_user_id
  // const isAdmin = state.user.role === USER_ROLE.ADMIN
  const releaseState = {
    ...state.release,
    currentUserId,
    isAdmin: state.user.is_admin,
  }

  return releaseState
}

const mapDispatch = () => {
  const service = new ReleaseService()

  return {
    async update(param) {
      return service.update(param)
    },
    async getList() {
      return service.list()
    },
    resetAll() {
      return service.resetList()
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
