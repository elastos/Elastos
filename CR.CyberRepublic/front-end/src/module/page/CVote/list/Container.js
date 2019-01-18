import { createContainer, api_request } from '@/util'
import Component from './Component'
import { COUNCIL_MEMBER_IDS, USER_ROLE } from '@/constant'

export default createContainer(Component, state => ({
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,

  // TODO: this should be dynamic
  isCouncil: COUNCIL_MEMBER_IDS.indexOf(state.user.current_user_id) >= 0 || state.user.role === USER_ROLE.COUNCIL,
  canCreate: COUNCIL_MEMBER_IDS.indexOf(state.user.current_user_id) >= 0 || state.user.role === USER_ROLE.COUNCIL || state.user.role === USER_ROLE.SECRETARY,
  language: state.language.language,
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
