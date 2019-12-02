import { message } from 'antd'
import { createContainer } from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import I18N from '@/I18N'

export default createContainer(Component, (state) => {
  return {
    isLogin: state.user.is_login,
    profile: state.user.profile
  }
}, () => {
  const userService = new UserService()
  return {
    async logout () {
      const rs = await userService.logout()
      if (rs) {
        message.success(I18N.get('logout.success'))
        userService.path.push('/login')
      }
    }
  }
})
