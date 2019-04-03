import {createContainer} from '@/util'
import Component from './Component'
import TeamService from '@/service/TeamService'

export default createContainer(Component, (state) => {
  let page = 'PUBLIC' // default

  if (/^\/admin/.test(state.router.location.pathname)) {
    page = 'ADMIN'
  } else if (/^\/profile/.test(state.router.location.pathname)) {
    page = 'LEADER'
  }

  return {
    is_admin: state.user.is_admin,
    is_login: state.user.is_login,
    current_user_id: state.user.current_user_id,
    page,
    loading: state.team.loading
  }
}, () => {

  const teamService = new TeamService()

  return {
    async getTeamDetail(teamId) {
      return teamService.get(teamId)
    },

    resetTeamDetail() {
      return teamService.resetTeamDetail()
    },

    async deleteTeam(teamId) {
      return teamService.deleteTeam(teamId)
    }
  }
})
