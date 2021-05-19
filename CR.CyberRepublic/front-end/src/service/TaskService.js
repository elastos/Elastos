import BaseService from '../model/BaseService'
import _ from 'lodash'
import { api_request } from '@/util'

import { TASK_CANDIDATE_STATUS, TASK_STATUS } from '@/constant'

export default class extends BaseService {
  async list(filter = {}) {
    const result = await api_request({
      path: '/api/task/list',
      method: 'get',
      data: filter,
    })

    return result
  }

  async loadMore(qry) {
    const taskRedux = this.store.getRedux('task')
    const path = '/api/task/list'

    const result = await api_request({
      path,
      method: 'get',
      data: qry,
    })

    const oldTasks = this.store.getState().task.all_tasks || []

    this.dispatch(taskRedux.actions.all_tasks_total_update(result.total))
    this.dispatch(taskRedux.actions.all_tasks_update(oldTasks.concat(_.values(result.list))))

    return result
  }

  async index(qry) {
    const taskRedux = this.store.getRedux('task')

    this.dispatch(taskRedux.actions.loading_update(true))

    const path = '/api/task/list'
    this.abortFetch(path)

    let result
    try {
      result = await api_request({
        path,
        method: 'get',
        data: qry,
        signal: this.getAbortSignal(path),
      })

      this.dispatch(taskRedux.actions.loading_update(false))
      this.dispatch(taskRedux.actions.all_tasks_reset())
      this.dispatch(taskRedux.actions.all_tasks_total_update(result.total))
      this.dispatch(taskRedux.actions.all_tasks_update(_.values(result.list)))
    } catch (e) {
      // Do nothing
    }

    return result
  }

  async get(taskId) {
    const taskRedux = this.store.getRedux('task')

    // sets redux store task.loading = true
    this.dispatch(taskRedux.actions.loading_update(true))

    const result = await api_request({
      path: `/api/task/${taskId}`,
      method: 'get',
    })

    this.dispatch(taskRedux.actions.loading_update(false))

    // Format data for dropdown select community
    if (result) {
      result.taskCommunity = []
      if (result.communityParent) {
        result.taskCommunity.push(result.communityParent._id)
      }

      if (result.community) {
        result.taskCommunity.push(result.community._id)
      }

      // Format data for dropdown select community -- end
      this.dispatch(taskRedux.actions.detail_update(result))

      return result
    }
  }

  /**
     * Need to rework this to return the updated doc, though this isn't a true
     * PUT the issue is it's just easier to have side effects
     *
     * @param taskId
     * @param doc
     * @returns {Promise<*>}
     */
  async update(taskId, doc) {
    const taskRedux = this.store.getRedux('task')

    this.dispatch(taskRedux.actions.loading_update(true))

    const result = await api_request({
      path: `/api/task/${taskId}`,
      method: 'put',
      data: doc,
    })

    const detail = {
      ...this.store.getState().task.detail,
      ...doc,
    }

    this.dispatch(taskRedux.actions.detail_reset())
    this.dispatch(taskRedux.actions.detail_update(detail))
    this.dispatch(taskRedux.actions.loading_update(false))

    return result
  }

  /**
     * Here we assume we are using task.detail in the store, so we
     * then push on to the task.detail.candidates array
     *
     * @param taskId
     * @param userId
     * @param teamId
     * @param applyMsg
     * @returns {Promise<*>}
     */
  async pushCandidate(taskId, userId, teamId, applyMsg, attachment, attachmentFilename, bid) {
    const taskRedux = this.store.getRedux('task')
    this.dispatch(taskRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/task/addCandidate',
      method: 'post',
      data: {
        taskId,
        userId,
        teamId,
        applyMsg,
        attachment,
        attachmentFilename,
        bid,
      },
    })

    const curTaskDetail = this.store.getState().task.detail
    curTaskDetail.candidates = curTaskDetail.candidates || []
    curTaskDetail.candidates.push(result)
    this.dispatch(taskRedux.actions.detail_update(curTaskDetail))

    this.dispatch(taskRedux.actions.loading_update(false))

    return result
  }

  async updateApplication(taskId, data) {
    const taskRedux = this.store.getRedux('task')
    this.dispatch(taskRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/task/updateCandidate',
      method: 'post',
      data,
    })

    this.dispatch(taskRedux.actions.loading_update(false))

    const curTaskDetail = this.store.getState().task.detail
    _.remove(curTaskDetail.candidates, { _id: data.taskCandidateId })
    curTaskDetail.candidates.push(result)

    const all_tasks = this.store.getState().task.all_tasks
    if (!_.isEmpty(all_tasks)) {
      const task = _.find(all_tasks, { _id: taskId })
      const ind = _.indexOf(_.values(all_tasks), task)

      all_tasks[ind].candidates = curTaskDetail.candidates
      this.dispatch(taskRedux.actions.all_tasks_update(all_tasks))
    }

    this.dispatch(taskRedux.actions.detail_update(curTaskDetail))

    return result
  }

  async register(taskId, userId) {
    const taskRedux = this.store.getRedux('task')
    this.dispatch(taskRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/task/register',
      method: 'post',
      data: {
        taskId,
        userId,
      },
    })

    this.dispatch(taskRedux.actions.loading_update(false))

    const curTaskDetail = this.store.getState().task.detail
    curTaskDetail.candidates = curTaskDetail.candidates || []
    curTaskDetail.candidates.push(result)

    const all_tasks = this.store.getState().task.all_tasks
    if (!_.isEmpty(all_tasks)) {
      const task = _.find(all_tasks, { _id: taskId })
      const ind = _.indexOf(_.values(all_tasks), task)

      all_tasks[ind].candidates = curTaskDetail.candidates
      this.dispatch(taskRedux.actions.all_tasks_update(all_tasks))
    }

    this.dispatch(taskRedux.actions.detail_update(curTaskDetail))

    return result
  }

  async pullCandidate(taskId, taskCandidateId) {
    const taskRedux = this.store.getRedux('task')
    const result = await api_request({
      path: '/api/task/removeCandidate',
      method: 'post',
      data: {
        taskId,
        taskCandidateId,
      },
    })

    const curTaskDetail = this.store.getState().task.detail

    _.remove(curTaskDetail.candidates, candidate => candidate._id === taskCandidateId)

    this.dispatch(taskRedux.actions.detail_update(curTaskDetail))

    return result
  }

  async deregister(taskId, taskCandidateId) {
    const taskRedux = this.store.getRedux('task')
    this.dispatch(taskRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/task/deregister',
      method: 'post',
      data: {
        taskId,
        taskCandidateId,
      },
    })

    this.dispatch(taskRedux.actions.loading_update(false))
    const curTaskDetail = this.store.getState().task.detail

    _.remove(curTaskDetail.candidates, candidate => candidate._id === taskCandidateId)

    const all_tasks = this.store.getState().task.all_tasks
    if (!_.isEmpty(all_tasks)) {
      const task = _.find(all_tasks, { _id: taskId })
      const ind = _.indexOf(_.values(all_tasks), task)

      all_tasks[ind].candidates = curTaskDetail.candidates
      this.dispatch(taskRedux.actions.all_tasks_update(all_tasks))
    }

    this.dispatch(taskRedux.actions.detail_update(curTaskDetail))

    return result
  }

  async acceptCandidate(taskCandidateId) {
    const taskRedux = this.store.getRedux('task')

    this.dispatch(taskRedux.actions.loading_update(true))

    const task = await api_request({
      path: '/api/task/acceptCandidate',
      method: 'post',
      data: {
        taskCandidateId,
      },
    })

    // we do this the hard way because the result doesn't have all the fields populated
    // TODO: should we populate everything?
    const curTaskDetail = this.store.getState().task.detail

    const acceptedCandidate = _.find(curTaskDetail.candidates, o => o._id === taskCandidateId)

    acceptedCandidate.status = TASK_CANDIDATE_STATUS.APPROVED

    if (task.status === TASK_STATUS.APPROVED) {
      curTaskDetail.status = TASK_STATUS.APPROVED
    }

    this.dispatch(taskRedux.actions.detail_update(curTaskDetail))
    this.dispatch(taskRedux.actions.loading_update(false))

    return task
  }

  async rejectCandidate(taskCandidateId) {
    const taskRedux = this.store.getRedux('task')

    this.dispatch(taskRedux.actions.loading_update(true))

    const task = await api_request({
      path: '/api/task/rejectCandidate',
      method: 'post',
      data: {
        taskCandidateId,
      },
    })

    const curTaskDetail = this.store.getState().task.detail
    const rejectedCandidate = _.find(curTaskDetail.candidates, o => o._id === taskCandidateId)
    rejectedCandidate.status = TASK_CANDIDATE_STATUS.REJECTED

    if (task.status === TASK_STATUS.ASSIGNED) {
      curTaskDetail.status = TASK_STATUS.ASSIGNED
    }

    this.dispatch(taskRedux.actions.loading_update(false))
    this.dispatch(taskRedux.actions.detail_update(curTaskDetail))

    return task
  }

  async withdrawCandidate(taskCandidateId) {
    const taskRedux = this.store.getRedux('task')

    this.dispatch(taskRedux.actions.loading_update(true))

    const task = await api_request({
      path: '/api/task/withdrawCandidate',
      method: 'post',
      data: {
        taskCandidateId,
      },
    })

    const curTaskDetail = this.store.getState().task.detail
    const withdrawnCandidate = _.find(curTaskDetail.candidates, o => o._id === taskCandidateId)
    curTaskDetail.candidates = _.without(curTaskDetail.candidates, withdrawnCandidate)

    this.dispatch(taskRedux.actions.loading_update(false))
    this.dispatch(taskRedux.actions.detail_update(curTaskDetail))

    return task
  }

  async markComplete(taskCandidateId) {
    const taskRedux = this.store.getRedux('task')
    const task = await api_request({
      path: '/api/task/markTaskComplete',
      method: 'post',
      data: {
        taskCandidateId,
      },
    })

    const curTaskDetail = this.store.getState().task.detail
    const candidate = _.find(curTaskDetail.candidates, o => o._id === taskCandidateId)
    candidate.complete = true

    this.dispatch(taskRedux.actions.detail_update(curTaskDetail))

    return task
  }

  resetAllTasks() {
    const taskRedux = this.store.getRedux('task')
    this.dispatch(taskRedux.actions.all_tasks_reset())
  }

  resetTaskDetail() {
    const taskRedux = this.store.getRedux('task')
    this.dispatch(taskRedux.actions.detail_reset())
  }

  async create(doc) {
    const taskRedux = this.store.getRedux('task')
    this.dispatch(taskRedux.actions.loading_update(true))

    const res = await api_request({
      path: '/api/task/create',
      method: 'post',
      data: doc,
    })

    this.dispatch(taskRedux.actions.loading_update(false))

    return res
  }

  async markVisited(taskCandidateId, owner) {
    const res = await api_request({
      path: '/api/task/markVisited',
      method: 'post',
      data: {
        taskCandidateId,
        owner,
      },
    })

    return res
  }

  async saveFilter(filter) {
    const taskRedux = this.store.getRedux('task')
    this.dispatch(taskRedux.actions.filter_update(filter))
  }
}
