import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'
import TeamService from '@/service/TeamService'
import _ from 'lodash'

import {USER_ROLE, TASK_STATUS, TASK_CANDIDATE_STATUS, TASK_TYPE, TASK_CATEGORY} from '@/constant'

export default createContainer(Component, (state) => {
  const currentUserId = state.user.current_user_id

  let page = 'PUBLIC' // default

  if (/^\/admin/.test(state.router.location.pathname)) {
    page = 'ADMIN'
  } else if (/^\/profile/.test(state.router.location.pathname)) {
    page = 'LEADER'
  }

  return {
    ...state.task,
    ...state.team,
    page,
    currentUserId,
    is_leader: state.user.role === USER_ROLE.LEADER,
    is_admin: state.user.is_admin,
    loading: state.task.loading || state.team.loading
  }
}, () => {

  const taskService = new TaskService()

  return {

    /**
         * We are querying tasks:
         *
         * 1. owner of
         * 2. we are a candidate of
         * 3. assigned to (this is a candidate with STATUS approved) including in 2
         *
         * @returns {Promise<*>}
         */
    async getTasks(query) {
      return taskService.index({
        type: TASK_TYPE.PROJECT,
        ...query,
      })
    },

    async loadMoreProjects(query) {
      return taskService.loadMore({
        type: TASK_TYPE.PROJECT,
        ...query,
      })
    },

    resetTasks () {
      return taskService.resetAllTasks()
    },

    async setFilter(options) {

    },

    async getUserTeams(currentUserId) {
      const teamService = new TeamService()
      return teamService.getUserTeams(currentUserId)
    }
  }
})
