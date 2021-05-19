import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'
import TeamService from '@/service/TeamService'

export default createContainer(Component, (state) => {
  return {
    ...state.task,
    ownedTeams: state.team.all_teams,
    userId: state.user.current_user_id,
    currentUserAvatar: state.user.profile.avatar,
    is_login: state.user.is_login,
    is_admin: state.user.is_admin
  }
}, () => {
  const taskService = new TaskService()
  const teamService = new TeamService()

  return {
    async updateApplication(taskId, data) {
      return taskService.updateApplication(taskId, data)
    },

    async getTeams(query) {
      return teamService.index(query)
    },

    resetTeams() {
      return teamService.resetAllTeams()
    }
  }
})
