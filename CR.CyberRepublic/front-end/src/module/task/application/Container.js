import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'
import TeamService from '@/service/TeamService'
import { message } from 'antd/lib/index'
import { logger } from '@/util'

export default createContainer(Component, (state) => {
  return {
    userId: state.user.current_user_id,
    is_login: state.user.is_login,
    is_admin: state.user.is_admin
  }
}, () => {

  const teamService = new TeamService()
  const taskService = new TaskService()

  return {
    async listTeamsOwned(userId) {
      try {
        const result = await teamService.index({
          owner: userId
        })

        return result
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async getTaskDetail(taskId) {
      return taskService.get(taskId)
    },

    async markVisited (taskCandidateId, owner) {
      return taskService.markVisited(taskCandidateId, owner)
    },

    resetTaskDetail() {
      return taskService.resetTaskDetail()
    },

    async markComplete(taskCandidateId) {
      return taskService.markComplete(taskCandidateId)
    },

    async pullCandidate(taskId, taskCandidateId) {
      try {
        const result = await taskService.pullCandidate(taskId, taskCandidateId)
        return result
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async acceptCandidate(taskCandidateId) {
      try {
        const result = await taskService.acceptCandidate(taskCandidateId)
        return result
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    }
  }
})
