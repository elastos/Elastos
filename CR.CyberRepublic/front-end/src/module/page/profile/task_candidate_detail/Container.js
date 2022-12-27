import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'
import { message } from 'antd/lib/index'

export default createContainer(Component, (state) => {
  let page = 'PUBLIC' // default

  if (/^\/admin/.test(state.router.location.pathname)) {
    page = 'ADMIN'
  } else if (/^\/profile/.test(state.router.location.pathname)) {
    page = 'LEADER'
  }

  return {
    task: state.task.detail,
    loading: state.task.loading,
    page
  }
}, () => {
  const taskService = new TaskService()

  return {
    async getTaskDetail (taskId) {
      return taskService.get(taskId)
    },

    resetTaskDetail () {
      return taskService.resetTaskDetail()
    }
  }
})
