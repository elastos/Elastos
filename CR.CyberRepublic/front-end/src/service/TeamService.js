import BaseService from '../model/BaseService'
import _ from 'lodash'
import { api_request } from '@/util'
import { TEAM_TYPE } from '@/constant'

export default class extends BaseService {
  async index(qry = {}) {
    const teamRedux = this.store.getRedux('team')

    this.dispatch(teamRedux.actions.loading_update(true))

    const path = '/api/team/list'
    this.abortFetch(path)

    let result
    try {
      result = await api_request({
        path,
        method: 'get',
        data: qry,
        signal: this.getAbortSignal(path),
      })

      await this.dispatch(teamRedux.actions.all_teams_reset())
      await this.dispatch(teamRedux.actions.all_teams_total_update(result.total))
      await this.dispatch(teamRedux.actions.all_teams_update(result.list))
      await this.dispatch(teamRedux.actions.loading_update(false))
    } catch (e) {
      // Do nothing
    }

    return result
  }

  async loadMore(qry = {}) {
    const teamRedux = this.store.getRedux('team')
    const path = '/api/team/list'

    const result = await api_request({
      path,
      method: 'get',
      data: qry,
    })

    const oldTeams = this.store.getState().team.all_teams || []

    await this.dispatch(teamRedux.actions.all_teams_total_update(result.total))
    await this.dispatch(teamRedux.actions.all_teams_update(oldTeams.concat(_.values(result.list))))

    return result
  }

  async loadAllCircles(qry = {}) {
    const teamRedux = this.store.getRedux('team')

    this.dispatch(teamRedux.actions.all_circles_loading_update(true))

    const result = await api_request({
      path: '/api/team/list',
      method: 'get',
      data: {
        includeTasks: true,
        ...qry,
        type: TEAM_TYPE.CRCLE,
      },
    })

    this.dispatch(teamRedux.actions.all_circles_reset())
    this.dispatch(teamRedux.actions.all_circles_update(result.list))
    this.dispatch(teamRedux.actions.all_circles_loading_update(false))

    return result
  }

  async getUserTeams(userId) {
    const teamRedux = this.store.getRedux('team')

    this.dispatch(teamRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/team/list',
      method: 'get',
      data: {
        teamHasUser: userId,
      },
    })

    this.dispatch(teamRedux.actions.all_teams_reset())
    this.dispatch(teamRedux.actions.all_teams_update(result.list))
    this.dispatch(teamRedux.actions.loading_update(false))

    return result
  }

  async get(teamId) {
    const teamRedux = this.store.getRedux('team')
    this.dispatch(teamRedux.actions.loading_update(true))

    const result = await api_request({
      path: `/api/team/${teamId}`,
      method: 'get',
    })

    this.dispatch(teamRedux.actions.loading_update(false))
    this.dispatch(teamRedux.actions.detail_update(result))

    return result
  }

  async update(param) {
    const teamRedux = this.store.getRedux('team')
    this.dispatch(teamRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/team/update',
      method: 'post',
      data: param,
    })

    const detail = {
      ...this.store.getState().team.detail,
      ...param,
    }

    this.dispatch(teamRedux.actions.detail_reset())
    this.dispatch(teamRedux.actions.detail_update(detail))
    this.dispatch(teamRedux.actions.loading_update(false))

    return result
  }

  async create(param) {
    const teamRedux = this.store.getRedux('team')
    this.dispatch(teamRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/team/create',
      method: 'post',
      data: param,
    })

    const allTeams = this.store.getState().team.all_teams || []
    allTeams[_.size(_.values(allTeams))] = result
    this.dispatch(teamRedux.actions.loading_update(false))
    this.dispatch(teamRedux.actions.all_teams_update(allTeams))

    return result
  }

  resetAllTeams() {
    const teamRedux = this.store.getRedux('team')
    this.dispatch(teamRedux.actions.all_teams_reset())
  }

  resetTeamDetail() {
    const teamRedux = this.store.getRedux('team')
    this.dispatch(teamRedux.actions.detail_reset())
  }

  async pushCandidate(teamId, userId, applyMsg) {
    const teamRedux = this.store.getRedux('team')
    this.dispatch(teamRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/team/addCandidate',
      method: 'post',
      data: {
        userId,
        teamId,
        applyMsg,
      },
    })

    this.dispatch(teamRedux.actions.loading_update(false))

    const curTeamDetail = this.store.getState().team.detail
    curTeamDetail.members.push(result)

    if (curTeamDetail.type === TEAM_TYPE.CRCLE) {
      const userRedux = this.store.getRedux('user')
      const curUserDetail = this.store.getState().user
      curUserDetail.circles = _.values(curUserDetail.circles) || []
      curUserDetail.circles.push(curTeamDetail)

      this.dispatch(userRedux.actions.circles_update(curUserDetail.circles))
    }

    this.dispatch(teamRedux.actions.detail_update(curTeamDetail))

    return result
  }

  async acceptCandidate(teamCandidateId) {
    const teamRedux = this.store.getRedux('team')
    this.dispatch(teamRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/team/action/accept',
      method: 'post',
      data: {
        teamCandidateId,
      },
    })

    this.dispatch(teamRedux.actions.loading_update(false))

    const curTeamDetail = this.store.getState().team.detail
    const member = _.find(curTeamDetail.members, { _id: teamCandidateId })
    member.status = result.status
    this.dispatch(teamRedux.actions.detail_update(curTeamDetail))

    return result
  }

  async rejectCandidate(teamCandidateId) {
    const teamRedux = this.store.getRedux('team')
    this.dispatch(teamRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/team/action/reject',
      method: 'post',
      data: {
        teamCandidateId,
      },
    })

    this.dispatch(teamRedux.actions.loading_update(false))

    const curTeamDetail = this.store.getState().team.detail
    const member = _.find(curTeamDetail.members, { _id: teamCandidateId })
    member.status = result.status
    this.dispatch(teamRedux.actions.detail_update(curTeamDetail))

    return result
  }

  async withdrawCandidate(teamCandidateId) {
    const teamRedux = this.store.getRedux('team')
    this.dispatch(teamRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/team/action/withdraw',
      method: 'post',
      data: {
        teamCandidateId,
      },
    })

    const curTeamDetail = this.store.getState().team.detail

    if (curTeamDetail.type === TEAM_TYPE.CRCLE) {
      const userRedux = this.store.getRedux('user')
      const curUserDetail = this.store.getState().user
      curUserDetail.circles = _.values(curUserDetail.circles) || []
      curUserDetail.circles = _.filter(curUserDetail.circles, circle => circle._id !== curTeamDetail._id)

      this.dispatch(userRedux.actions.circles_update(curUserDetail.circles))
    }

    const member = _.find(curTeamDetail.members, { _id: teamCandidateId })
    curTeamDetail.members = _.without(curTeamDetail.members, member)
    this.dispatch(teamRedux.actions.detail_update(curTeamDetail))
    this.dispatch(teamRedux.actions.loading_update(false))

    return result
  }

  async deleteTeam(teamId) {
    const teamRedux = this.store.getRedux('team')
    this.dispatch(teamRedux.actions.loading_update(true))

    await api_request({
      path: '/api/team/action/delete',
      method: 'post',
      data: {
        teamId,
      },
    })

    this.dispatch(teamRedux.actions.loading_update(false))
    this.dispatch(teamRedux.actions.detail_update({}))
  }
}
