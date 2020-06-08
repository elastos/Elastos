import { createContainer } from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'

export default createContainer(Component, (state) => {
  return {
    is_login: state.user.is_login,
    currentUserId: state.user.current_user_id,
    popup_update: state.user.popup_update,
    user: state.user
  }
}, () => {
  const userService = new UserService()
  return {
    async getCurrentUser() {
      try {
        await userService.getCurrentUser()
      } catch (err) {
        console.log(err)
      }
    },

    async updateUserPopupUpdate(userId) {
      await userService.update(userId, {
        popupUpdate: true
      })
    }
  }
})
