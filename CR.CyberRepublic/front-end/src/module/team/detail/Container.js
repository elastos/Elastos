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
    ...state.team,
    page,
    currentUserId: state.user.current_user_id
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

    async applyToTeam(teamId, userId, applyMsg) {
      return teamService.pushCandidate(teamId, userId, applyMsg)
    },

    async acceptCandidate(teamCandidateId) {
      return teamService.acceptCandidate(teamCandidateId)
    },

    async rejectCandidate(teamCandidateId) {
      return teamService.rejectCandidate(teamCandidateId)
    },

    async withdrawCandidate(teamCandidateId) {
      return teamService.withdrawCandidate(teamCandidateId)
    }
  }
})
