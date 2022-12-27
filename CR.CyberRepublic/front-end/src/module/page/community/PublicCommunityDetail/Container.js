import {createContainer} from '@/util'
import Component from './Component'
import CommunityService from '@/service/CommunityService'
import UserService from '@/service/UserService'
import TaskService from '@/service/TaskService'
import _ from 'lodash'
import {TASK_CATEGORY, TASK_TYPE} from '@/constant'

export default createContainer(Component, (state, ownProps) => {
  const taskState = state.task

  if (!_.isArray(state.task.all_tasks)) {
    taskState.all_tasks = _.values(state.task.all_tasks)
  }

  taskState.events = _.filter(taskState.all_tasks, {type: TASK_TYPE.EVENT})
  taskState.tasks = _.filter(taskState.all_tasks, {type: TASK_TYPE.TASK})

  return {
    ...taskState,
    current_user_id: state.user.current_user_id
  }
}, () => {

  const communityService = new CommunityService()
  const userService = new UserService()
  const taskService = new TaskService()

  return {
    async getAllCountryCommunity () {
      return communityService.getAllCountryCommunities()
    },
    async getCommunityDetail(communityId) {
      return communityService.get(communityId)
    },
    async getSubCommunities(parentCommunityId) {
      return communityService.getSubCommunities(parentCommunityId)
    },
    async createSubCommunity(community) {
      return communityService.create(community)
    },
    async updateCommunity(community) {
      return communityService.update(community)
    },
    async getCommunityMembers(communityId) {
      return communityService.getMembers(communityId)
    },
    async getUserByIds (ids) {
      return userService.getByIds(ids)
    },
    async getSocialEvents () {
      return taskService.index({
        category: TASK_CATEGORY.SOCIAL
      })
    },
    resetTasks() {
      return taskService.resetAllTasks()
    },
    async addMember(memberId, communityId) {
      return communityService.addMember(memberId, communityId)
    },
    async removeMember(memberId, communityId) {
      return communityService.removeMember(memberId, communityId)
    }
  }
})
