import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'

export default createContainer(Component, (state, ownProps) => {
  return {
    task: state.task.detail,
    loading: state.task.loading,
    currentUserId: state.user.current_user_id
  }
}, () => {

  const taskService = new TaskService()
  return {
    async getTaskDetail(taskId) {
      return taskService.get(taskId)
    },

    resetTaskDetail() {
      return taskService.resetTaskDetail()
    },

    async register(taskId, userId) {
      return taskService.register(taskId, userId)
    },

    async deregister(taskId, taskCandidateId) {
      return taskService.deregister(taskId, taskCandidateId)
    }
  }
})
