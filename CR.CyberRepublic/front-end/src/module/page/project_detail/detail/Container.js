import { createContainer } from '@/util'
import TaskService from '@/service/TaskService'
import TeamService from '@/service/TeamService'
import CommentService from '@/service/CommentService'
import Component from './Component'
import { TASK_TYPE, TASK_CATEGORY, TEAM_TYPE } from '@/constant'
import _ from 'lodash'

export default createContainer(
  Component,
  state => {
    let page = 'CR100' // default

    if (/^\/admin/.test(state.router.location.pathname)) {
      page = 'ADMIN'
    } else if (/^\/profile/.test(state.router.location.pathname)) {
      page = 'LEADER'
    } else if (/^\/project-detail/.test(state.router.location.pathname)) {
      page = 'PUBLIC'
    }

    return {
      ...state.task,
      page,
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
    const sortedTasks = []

    return {
      async getTaskDetail(taskId) {
        return taskService.get(taskId)
      },

      resetTaskDetail() {
        return taskService.resetTaskDetail()
      },

      async getTeams(query) {
        return teamService.index({
          ...query,
          type: TEAM_TYPE.TEAM
        })
      },

      async getTasks(filters) {
        return taskService.index({
          ...filters,
          type: TASK_TYPE.PROJECT,
          category: TASK_CATEGORY.CR100
        })
      },

      resetTasks() {
        return taskService.resetAllTasks()
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
