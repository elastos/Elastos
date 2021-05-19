import {createContainer} from '@/util'
import Component from './Component'

export default createContainer(Component, (state) => {
  return {
    ...state.team,
    userId: state.user.current_user_id,
    is_login: state.user.is_login,
    is_admin: state.user.is_admin
  }
}, () => {
  return {
  }
})
