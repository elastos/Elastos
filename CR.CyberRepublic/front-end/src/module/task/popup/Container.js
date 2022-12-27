import {createContainer} from '@/util'
import TaskService from '@/service/TaskService'
import Component from './Component'
import _ from 'lodash'

export default createContainer(Component, (state) => {
  let page

  if (/^\/admin/.test(state.router.location.pathname)) {
    page = 'ADMIN'
  } else if (/^\/profile/.test(state.router.location.pathname)) {
    page = 'LEADER'
  }

  return {
    ...state.task,
    page,
    currentUserId: state.user.current_user_id,
    currentUserAvatar: state.user.profile.avatar,
    is_admin: state.user.is_admin,
    loading: state.task.loading
  }
}, () => {
  const taskService = new TaskService()

  return {
    async getTaskDetail(taskId) {
      return taskService.get(taskId)
    },

    resetTaskDetail() {
      return taskService.resetTaskDetail()
    }
  }
})
