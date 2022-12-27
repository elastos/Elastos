import {createContainer} from '@/util'
import TaskService from '@/service/TaskService'
import TeamService from '@/service/TeamService'
import Component from './Component'
import _ from 'lodash'

export default createContainer(Component, (state) => {
  let page = 'CR100' // default

  if (/^\/admin/.test(state.router.location.pathname)) {
    page = 'ADMIN'
  } else if (/^\/profile/.test(state.router.location.pathname)) {
    page = 'LEADER'
  }

  return {
    task: state.task.detail,
    page,
    ownedTeams: state.team.all_teams,
    currentUserId: state.user.current_user_id,
    currentUserAvatar: state.user.profile.avatar,
    is_admin: state.user.is_admin,
    loading: state.task.loading || state.team.loading
  }
}, () => {
  const taskService = new TaskService()
  const teamService = new TeamService()

  return {
    async getTaskDetail(taskId) {
      return taskService.get(taskId)
    },

    resetTaskDetail() {
      return taskService.resetTaskDetail()
    },

    // TODO: we need to get all the users's teams and all teams that applied for the task
    async getTeams(query) {
      return teamService.index(query)
    },

    async getUserTeams(userId) {
      return teamService.getUserTeams(userId)
    },

    async applyToTask(taskId, userId, teamId, applyMsg, attachment, attachmentFilename, bid) {

      return taskService.pushCandidate(taskId, userId, teamId, applyMsg, attachment, attachmentFilename, bid)
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
})
