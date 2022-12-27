import {createContainer} from '@/util'
import UserService from '@/service/UserService'
import Component from './Component'
import {TASK_TYPE, TASK_CATEGORY} from '@/constant'
import _ from 'lodash'

export default createContainer(Component, (state) => {
  return {
    ...state.task
  }
}, () => {
  const userService = new UserService()

  return {
    async getEmpowerUsers() {
      return userService.getAll({
        empower: JSON.stringify({$exists: true})
      })
    }
  }
})
