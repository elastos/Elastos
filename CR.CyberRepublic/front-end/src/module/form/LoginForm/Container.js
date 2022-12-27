import { createContainer, goPath } from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import { message } from 'antd'
import I18N from '@/I18N'

message.config({ top: 100 })

export default createContainer(
  Component,
  state => ({
    ...state.user.login_form,
    language: state.language
  }),
  () => {
    const userService = new UserService()

    return {
      async login(username, password, persist) {
        try {
          const rs = await userService.login(username.trim(), password, persist)

          if (rs) {
            message.success(`${I18N.get('login.success')}, ${username.trim()}`)

            const loginRedirect = sessionStorage.getItem('loginRedirect')
            if (loginRedirect) {
              this.history.push(loginRedirect)
              sessionStorage.setItem('loggedIn', '1')
              sessionStorage.setItem('loginRedirect', null)
            } else {
              this.history.push('/profile/info')
            }
            return true
          }
        } catch (err) {
          message.error(err.message)
          return false
        }
      },
      async loginElaUrl() {
        return await userService.loginElaUrl()
      },
      async checkElaAuth(qrcodeStr) {
        try {
          const rs = await userService.checkElaAuth(qrcodeStr)
          if (rs && rs.did) {
            message.info('请注册账号或用已有账号登陆')
          }
          if (rs && rs.success && rs.username) {
            message.success(`${I18N.get('login.success')}, ${rs.username}`)
            const loginRedirect = sessionStorage.getItem('loginRedirect')
            if (loginRedirect) {
              
              this.history.push(loginRedirect)
              sessionStorage.setItem('loggedIn', '1')
              sessionStorage.setItem('loginRedirect', null)
            } else {
              this.history.push('/profile/info')
            }
          }
          return rs
        } catch (err) {
          message.error(err.message)
          return false
        }
      }
    }
  }
)
