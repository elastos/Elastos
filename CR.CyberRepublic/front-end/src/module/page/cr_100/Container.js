import {createContainer} from '@/util'
import TaskService from '@/service/TaskService'
import Component from './Component'
import {TASK_TYPE, TASK_CATEGORY} from '@/constant'
import _ from 'lodash'

export default createContainer(Component, (state) => {
  return {
    ...state.task
  }
}, () => {
  const taskService = new TaskService()

  return {
    async getTasks(filters) {
      return taskService.index({
        ...filters,
        type: TASK_TYPE.PROJECT,
        category: TASK_CATEGORY.CR100
      })
    },

    resetTasks () {
      return taskService.resetAllTasks()
    }
  }
})
