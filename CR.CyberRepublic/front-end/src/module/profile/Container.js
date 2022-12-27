import _ from 'lodash'
import { message } from 'antd'
import { createContainer } from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import { logger } from '@/util'

export default createContainer(
  Component,
  (state, ownProps) => ({
    is_admin: state.user.is_admin,
    is_login: state.user.is_login,
    currentUserId: _.get(ownProps, 'user._id', state.user.current_user_id)
  }),
  (dispatch, ownProps) => {
    const userService = new UserService()

    return {
      async getCurrentUser() {
        const userId = _.get(ownProps, 'user._id')
        try {
          if (userId) {
            await userService.getMember(userId)
          } else {
            await userService.getCurrentUser()
          }
        } catch (err) {
          message.error(err.message)
          logger.error(err)
        }
      },

      async updateUser(userId, data) {
        await userService.update(userId, data)
      },

      async getElaUrl() {
        return await userService.getElaUrl()
      },

      async getNewActiveDid() {
        return await userService.getNewActiveDid()
      }
    }
  }
)
