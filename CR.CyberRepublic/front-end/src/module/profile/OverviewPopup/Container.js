import { createContainer } from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'

export default createContainer(Component, state => ({
  member: state.member.detail,
  loading: state.member.loading,
}), () => {
  const userService = new UserService()
  return {
    async getMember(userId) {
      return userService.getMember(userId)
    },
    resetMemberDetail() {
      return userService.resetMemberDetail()
    },
  }
})
