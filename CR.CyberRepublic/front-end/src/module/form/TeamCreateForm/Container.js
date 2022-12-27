import {createContainer, goPath} from '@/util'
import Component from './Component'
import TeamService from '@/service/TeamService'

export default createContainer(Component, (state) => {
  return {
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

    async create(param) {
      return teamService.create(param)
    },

    async update(param) {
      return teamService.update(param)
    }
  }
})
