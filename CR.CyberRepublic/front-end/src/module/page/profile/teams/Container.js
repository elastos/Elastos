import {createContainer} from '@/util'
import Component from './Component'
import TeamService from '@/service/TeamService'
import {TEAM_TYPE} from '@/constant'
import _ from 'lodash'

export default createContainer(Component, (state) => {
  return {
    ...state.team,
    currentUserId: state.user.current_user_id,
    all_teams: _.filter(state.team.all_teams, { type: TEAM_TYPE.TEAM }),
    is_admin: state.user.is_admin
  }
}, () => {
  const teamService = new TeamService()

  return {
    async getTeams(query) {
      return teamService.index({
        ...query,
        type: TEAM_TYPE.TEAM
      })
    },

    async loadMoreTeams(filters) {
      await teamService.loadMore({
        ...filters,
        type: TEAM_TYPE.TEAM
      })
    },

    resetTeams() {
      return teamService.resetAllTeams()
    }
  }
})
