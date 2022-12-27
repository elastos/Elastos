import {createContainer} from '@/util'
import TeamService from '@/service/TeamService'
import Component from './Component'

export default createContainer(Component, (state) => {
  let page

  if (/^\/admin/.test(state.router.location.pathname)) {
    page = 'ADMIN'
  } else if (/^\/profile/.test(state.router.location.pathname)) {
    page = 'LEADER'
  }

  return {
    ...state.team,
    page,
    currentUserId: state.user.current_user_id,
    currentUserAvatar: state.user.profile.avatar,
    is_admin: state.user.is_admin,
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
    }
  }
})
