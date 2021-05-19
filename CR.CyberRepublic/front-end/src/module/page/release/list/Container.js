import {
  createContainer,
} from '@/util'
import ReleaseService from '@/service/ReleaseService'
import Component from './Component'

const mapState = (state) => {
  const currentUserId = state.user.current_user_id
  // const isAdmin = state.user.role === USER_ROLE.ADMIN
  const {
    all_releases_loading: loading,
    all_releases: dataList,
    all_releases_total: total,
  } = state.release
  const releaseState = {
    ...state.release,
    dataList,
    total,
    loading,
    isAdmin: state.user.is_admin,
  }

  return releaseState
}

const mapDispatch = () => {
  const service = new ReleaseService()

  return {
    async getList() {
      return service.list()
    },
    resetAll() {
      return service.resetList()
    },
    deleteData(id) {
      return service.delete(id)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
