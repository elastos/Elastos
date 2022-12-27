import {createContainer} from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'

export default createContainer(Component, (state) => {
  return {
    loading: state.member.loading,
    member: state.member.detail
  }
}, () => {
  const userService = new UserService()

  return {
    async getMember(userId) {
      return userService.getMember(userId, {admin: true})
    },

    resetMemberDetail() {
      return userService.resetMemberDetail()
    }
  }
})
