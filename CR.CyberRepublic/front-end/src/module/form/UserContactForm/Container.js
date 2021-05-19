import {createContainer, goPath} from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import {message} from 'antd'
import _ from 'lodash'

message.config({
  top: 100
})


export default createContainer(Component, (state) => {
  return {
    currentUserId: state.user.current_user_id,
    is_admin: state.user.is_admin
  }
}, () => {
  const userService = new UserService()

  return {
    async sendEmail(toUserId, formData) {
      return userService.sendEmail(this.currentUserId, toUserId, formData)
    },
  }
})
