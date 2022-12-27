import { createContainer } from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'

export default createContainer(Component, state => ({
  is_admin: state.user.is_admin,
  loading: state.member.loading,
}), () => {
  const userService = new UserService()

  return {
    async updateUser(formData, state) {
      // TODO: refactor this, if it's current user it's current_user_id and otherwise it's _id
      // should always be _id
      const userId = this.user.current_user_id || this.user._id
      const doc = {
        profile: {
          // General
          firstName: formData.firstName,
          lastName: formData.lastName,
          bio: formData.bio,
        },
      }
      await userService.updateRole(userId, { role: formData.role })

      return userService.update(userId, doc)
    },
    async updateAvatar(doc) {
      const userId = this.user.current_user_id || this.user._id
      return userService.update(userId, doc)
    },
  }
})
