import { createContainer } from '@/util'
import Component from './Component'
import ElipService from '@/service/ElipService'

export default createContainer(
  Component,
  state => ({
    user: state.user,
    isLogin: state.user.is_login
  }),
  () => {
    const service = new ElipService()

    return {
      createElip(elip) {
        return service.create(elip)
      }
    }
  }
)
