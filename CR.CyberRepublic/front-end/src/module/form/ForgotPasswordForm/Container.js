import {createContainer, goPath} from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import {message} from 'antd'
import I18N from '@/I18N'
import { logger } from '@/util'

message.config({
  top: 100
})

export default createContainer(Component, (state) => {
  return {
    ...state.user.login_form,
    language: state.language
  }
}, () => {
  const userService = new UserService()

  return {
    async forgotPassword(email) {
      try {
        const rs = await userService.forgotPassword(email)

        if (rs) {
          message.success(I18N.get('forgot.sent_email'))
        }

      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    }
  }
})
