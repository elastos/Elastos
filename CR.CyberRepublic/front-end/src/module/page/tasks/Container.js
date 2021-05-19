import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'

export default createContainer(Component, () => {
  return {}
}, () => {
  const taskService = new TaskService()

  return {
    async fetchTaskList() {
      const data = await taskService.list()
      return data
    }
  }
})
