import { createContainer, api_request } from '@/util'
import Component from './Component'

export default createContainer(Component, state => ({
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary,
  isCouncil: state.user.is_council,
  canManage: state.user.is_secretary || state.user.is_council,
}), () => ({

  async listData(param, isCouncil) {
    let result

    if (isCouncil) {
      result = await api_request({
        path: '/api/cvote/list',
        method: 'get',
        data: param,
      });
    } else {
      result = await api_request({
        path: '/api/cvote/list_public',
        method: 'get',
        data: param,
      });
    }

    return result;
  },
}))
