import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'
import {message} from 'antd'
import {TASK_STATUS} from '@/constant'
import { logger } from '@/util'

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
    page
  }
}, () => {

  const taskService = new TaskService()

  return {
    async getTaskDetail(taskId) {
      return taskService.get(taskId)
    },

    resetTaskDetail() {
      return taskService.resetTaskDetail()
    },

    async approveTask(taskId) {
      try {
        await taskService.update(taskId, {
          status: TASK_STATUS.APPROVED
        })
        message.success('Task approved successfully')
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    // TODO: this doesn't work since assignSelf only works on creation
    async approveAndAssignTask(taskId) {
      try {
        await taskService.update(taskId, {
          status: TASK_STATUS.APPROVED,
          assignSelf: true
        })
        message.success('Task approved and assigned successfully')
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async markAsSubmitted(taskId) {

      try {
        await taskService.update(taskId, {
          status: TASK_STATUS.SUBMITTED
        })

        message.success('Task marked as complete')
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async markAsDisbursed(taskId) {

      try {
        await taskService.update(taskId, {
          status: TASK_STATUS.DISTRIBUTED
        })

        message.success('Task marked as ELA disbursed')
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    // TODO: language here needs work
    async markAsSuccessful(taskId) {

      try {
        await taskService.update(taskId, {
          status: TASK_STATUS.SUCCESS
        })

        message.success('Task completion accepted')
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async archive(taskId) {
      try {
        await taskService.update(taskId, {
          archived: true
        })

        message.success('Task archived')
        window.history.back()
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async forceStart(taskId) {

      try {
        await taskService.update(taskId, {
          status: TASK_STATUS.ASSIGNED
        })

        message.success('Task marked as assigned')
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    }
  }
})
