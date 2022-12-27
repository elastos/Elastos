import {createContainer} from '@/util'
import Component from './Component'
import { message } from 'antd/lib/index'
import UserService from '@/service/UserService'
import { logger } from '@/util'
let userService

export default createContainer(Component, (state) => {
  return {
    users: state.member.users,
    users_total: state.member.users_total,
    loading: state.member.users_loading
  }
}, () => {
  userService = userService || new UserService()

  return {
    async listUsers (query) {
      try {
        return await userService.getAll(query)
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    }
  }
})
