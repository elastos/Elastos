import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'

export default createContainer(Component, (state) => {
  return {
    task: state.task.detail,
    loading: state.task.loading
  }
}, () => {
  const taskService = new TaskService()

  return {
    async getTaskDetail(taskId) {
      return taskService.get(taskId)
    },

    resetTaskDetail() {
      return taskService.resetTaskDetail()
    }
  }
})
