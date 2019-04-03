import { createContainer } from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import CommentService from '@/service/CommentService'
import TeamService from '@/service/TeamService'
import TaskService from '@/service/TaskService'

export default createContainer(Component, state => ({
  currentUserId: state.user.current_user_id,
  is_login: state.user.is_login,
  loading: state.member.loading,
  subscribing: state.member.subscribing,
  member: state.member.detail,
  teams: state.team.all_teams,
  tasks: state.task.all_tasks,
  loadingList: state.team.loading || state.task.loading,
}), () => {
  const userService = new UserService()
  const commentService = new CommentService()
  const teamService = new TeamService()
  const taskService = new TaskService()

  return {
    async getMember(userId) {
      return userService.getMember(userId)
    },

    resetMemberDetail() {
      return userService.resetMemberDetail()
    },

    async subscribe(type, parentId) {
      await commentService.subscribe(type, parentId, 'member')
    },

    async unsubscribe(type, parentId) {
      await commentService.unsubscribe(type, parentId, 'member')
    },

    async getUserTeams(currentUserId) {
      return teamService.getUserTeams(currentUserId)
    },

    async getTeams(query) {
      return teamService.index({
        ...query,
      })
    },

    resetTeams() {
      return teamService.resetAllTeams()
    },

    async getTasks(currentUserId) {
      return taskService.index({
        profileListFor: currentUserId,
      })
    },

    resetTasks() {
      return taskService.resetAllTasks()
    },
  }
})
