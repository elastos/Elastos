import { createContainer } from '@/util'
import Component from './Component'
import TeamService from '@/service/TeamService'
import TaskService from '@/service/TaskService'
import _ from 'lodash'

export default createContainer(
  Component,
  state => {
    return {
      currentUserId: state.user.current_user_id,
      currentUserAvatar: state.user.profile.avatar,
      is_admin: state.user.is_admin,
      is_login: state.user.is_login,
      ownedTeams: state.team.all_teams,
      loading: state.task.loading || state.team.loading
    }
  },
  () => {
    const teamService = new TeamService()
    const taskService = new TaskService()

    return {
      async getTeams(query) {
        return teamService.index(query)
      },

      resetAllTeams() {
        return teamService.resetAllTeams()
      },

      async applyToTask(
        taskId,
        userId,
        teamId,
        applyMsg,
        attachment,
        attachmentFilename,
        bid
      ) {
        return taskService.pushCandidate(
          taskId,
          userId,
          teamId,
          applyMsg,
          attachment,
          attachmentFilename,
          bid
        )
      },

      async createTeam(param) {
        return teamService.create(param)
      }
    }
  }
)
