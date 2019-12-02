import { message } from 'antd'
import { createContainer } from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import LanguageService from '@/service/LanguageService'
import I18N from '@/I18N'

export default createContainer(Component, state => ({
  isLogin: state.user.is_login,
  role: state.user.role,
  pathname: state.router.location.pathname,
  user: state.user,
}), () => {
  const userService = new UserService()
  const languageService = new LanguageService()
  return {
    async logout() {
      const rs = await userService.logout()
      if (rs) {
        message.success(I18N.get('logout.success'))
        userService.path.push('/login')
      }
    },
    changeLanguage(lang) {
      languageService.changeLanguage(lang)
    },
  }
})
