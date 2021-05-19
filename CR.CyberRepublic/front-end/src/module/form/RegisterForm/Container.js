import { createContainer, goPath } from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import { message } from 'antd'

message.config({ top: 100 })

/**
 * Note at the moment we do lazy client side registration code generation
 * TODO: move this to server side
 */
export default createContainer(
  Component,
  (state) => {
    return {
      ...state.user.register_form,
      language: state.language
    }
  },
  () => {
    const userService = new UserService()

    return {
      async register(username, password, did, email) {
        try {
          const rs = await userService.register(username, password, did, email)
          if (rs) {
            const registerRedirect = sessionStorage.getItem('registerRedirect')

            if (registerRedirect) {
              return true
            }
            this.history.push('/')
          }
        } catch (err) {
          message.error(
            err && err.message
              ? err.message
              : 'Registration Failed - Please Contact Our Support'
          )
        }
      },

      async sendEmail(toUserId, formData) {
        return userService.sendEmail(this.currentUserId, toUserId, formData)
      },

      async sendRegistrationCode(email, code) {
        return userService.sendRegistrationCode(email, code)
      },

      async checkEmail(email) {
        const rs = await userService.checkEmail(email)
        if (rs) {
          return rs.isExist === true ? true : false
        }
      }
    }
  }
)
