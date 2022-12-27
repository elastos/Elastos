import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'
import TeamService from '@/service/TeamService'
import _ from 'lodash'
import {TASK_CATEGORY, TASK_TYPE, TEAM_TYPE} from '@/constant'

let taskService
let teamService

export default createContainer(Component, (state) => {

  // TODO: not sure this should be all one struct
  return {
    ...state.task,
    ...state.team,
    ...state.user,
    all_circles: _.values(state.team.all_circles),
    loading: state.team.loading || state.task.loading
  }
}, () => {
  taskService = taskService || new TaskService()
  teamService = teamService || new TeamService()

  return {
    async getProjects(filters) {
      await taskService.index({
        ...filters,
        assignSelf: false,
        type: TASK_TYPE.PROJECT,
        category: TASK_CATEGORY.DEVELOPER
      })
    },

    async loadMoreProjects(filters) {
      await taskService.loadMore({
        ...filters,
        assignSelf: false,
        type: TASK_TYPE.PROJECT,
        category: TASK_CATEGORY.DEVELOPER
      })
    },

    async getTasks(filters) {
      await taskService.index({
        category: [TASK_CATEGORY.DEVELOPER, TASK_CATEGORY.SOCIAL, TASK_CATEGORY.GENERAL],
        ...filters,
        assignSelf: false,
        type: [TASK_TYPE.TASK, TASK_TYPE.EVENT]
      })
    },

    async loadMoreTasks(filters) {
      await taskService.loadMore({
        category: [TASK_CATEGORY.DEVELOPER, TASK_CATEGORY.SOCIAL, TASK_CATEGORY.GENERAL],
        ...filters,
        assignSelf: false,
        type: [TASK_TYPE.TASK, TASK_TYPE.EVENT]
      })
    },

    async getTeams(filters) {
      await teamService.index({
        ...filters,
        type: TEAM_TYPE.TEAM
      })
    },

    async loadMoreTeams(filters) {
      await teamService.loadMore({
        ...filters,
        type: TEAM_TYPE.TEAM
      })
    },

    async loadAllCircles() {
      return teamService.loadAllCircles({ includeTasks: false })
    },

    resetTasks () {
      return taskService.resetAllTasks()
    },

    resetTeams () {
      return teamService.resetAllTeams()
    }
  }
})
