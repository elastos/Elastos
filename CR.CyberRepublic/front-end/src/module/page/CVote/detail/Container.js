import { createContainer, api_request } from '@/util'
import Component from './Component'
import _ from 'lodash'
import { avatar_map } from '@/constant'

export default createContainer(Component, state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary,
  isCouncil: state.user.is_council,
  canManage: state.user.is_secretary || state.user.is_council,
  avatar_map,
  trackingStatus: _.get(_.last(state.cvoteTracking.all_public), 'status'),
  summaryStatus: _.get(_.last(state.cvoteSummary.all_public), 'status'),
}), () => ({
  async getData(id) {
    const d = await api_request({
      path: `/api/cvote/get/${id}`,
    })

    return d
  },
  async createCVote(param) {
    const rs = await api_request({
      path: '/api/cvote/create',
      method: 'post',
      data: param,
    })
    return rs
  },
  async updateCVote(param) {
    const rs = await api_request({
      path: '/api/cvote/update',
      method: 'post',
      data: param,
    })
    return rs
  },
  async vote(param) {
    const rs = await api_request({
      path: '/api/cvote/vote',
      method: 'post',
      data: param,
    })
    return rs
  },
  async finishCVote(param) {
    const rs = await api_request({
      path: '/api/cvote/finish',
      method: 'get',
      data: param,
    })
    return rs
  },
  async updateNotes(param) {
    const rs = await api_request({
      path: '/api/cvote/update_notes',
      method: 'post',
      data: param,
    })
    return rs
  },
}))
