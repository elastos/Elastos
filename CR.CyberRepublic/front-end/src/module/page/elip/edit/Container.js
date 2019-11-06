import { createContainer } from '@/util'
import Component from './Component'
import ElipService from '@/service/ElipService'

export default createContainer(
  Component,
  state => ({
    user: state.user,
    isLogin: state.user.is_login,
    currentUserId: state.user.current_user_id
  }),
  () => {
    const service = new ElipService()
    return {
      async getData(param) {
        return service.getData(param)
      },
      async resetData() {
        return service.resetData()
      },
      async update(param) {
        return service.update(param)
      }
    }
  }
)
