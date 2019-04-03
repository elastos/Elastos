import {createContainer} from '@/util'
import TeamService from '@/service/TeamService'
import TaskService from '@/service/TaskService'
import Component from './Component'
import _ from 'lodash'

export default createContainer(Component, (state) => {
  return {
    team: state.team,
    currentUserId: state.user.current_user_id,
    is_login: state.user.is_login,
    myCircles: state.user.circles,
    task_loading: state.task.loading,
    all_tasks: _.values(state.task.all_tasks),
    loading: state.team.loading
  }
}, () => {
  const teamService = new TeamService()
  const taskService = new TaskService()

  return {
    async getTeamDetail(teamId) {
      return teamService.get(teamId)
    },
    resetTeamDetail() {
      return teamService.resetTeamDetail()
    },
    async getCircleTasks(circleId) {
      return taskService.index({
        circle: circleId,
        assignSelf: false
      })
    },
    async applyToTeam(teamId, userId, applyMsg = 'I am interested in this CRcle.') {
      return teamService.pushCandidate(teamId, userId, applyMsg)
    },
    async withdrawCandidate(teamCandidateId) {
      return teamService.withdrawCandidate(teamCandidateId)
    }
  }
})
