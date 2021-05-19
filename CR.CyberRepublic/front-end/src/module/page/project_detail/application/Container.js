import { createContainer } from '@/util'
import TaskService from '@/service/TaskService'
import TeamService from '@/service/TeamService'
import CommentService from '@/service/CommentService'
import Component from './Component'

export default createContainer(
  Component,
  state => {
    return {
      ...state.task,
      ownedTeams: state.team.all_teams,
      currentUserId: state.user.current_user_id,
      currentUserAvatar: state.user.profile.avatar,
      is_admin: state.user.is_admin,
      is_login: state.user.is_login,
      loading: state.task.loading || state.team.loading
    }
  },
  () => {
    const taskService = new TaskService()
    const teamService = new TeamService()
    const commentService = new CommentService()

    return {
      async getTaskDetail(taskId) {
        return taskService.get(taskId)
      },

      resetTaskDetail() {
        return taskService.resetTaskDetail()
      },

      async getTeams(query) {
        return teamService.index(query)
      },

      async applyToTask(
        taskId,
        userId,
        teamId,
        applyMsg,
        attachment,
        attachmentFilename
      ) {
        return taskService.pushCandidate(
          taskId,
          userId,
          teamId,
          applyMsg,
          attachment,
          attachmentFilename
        )
      },

      async updateApplication(taskId, data) {
        return taskService.updateApplication(taskId, data)
      },

      async subscribeToProject(taskId) {
        return commentService.subscribe('task', taskId)
      },

      async unsubscribeFromProject(taskId) {
        return commentService.unsubscribe('task', taskId)
      },

      resetAllTeams() {
        return teamService.resetAllTeams()
      },

      async acceptCandidate(taskCandidateId) {
        return taskService.acceptCandidate(taskCandidateId)
      },

      async rejectCandidate(taskCandidateId) {
        return taskService.rejectCandidate(taskCandidateId)
      },

      async withdrawCandidate(taskCandidateId) {
        return taskService.withdrawCandidate(taskCandidateId)
      }
    }
  }
)
