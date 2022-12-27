import { createContainer } from '@/util'
import Component from './Component'
import { USER_ROLE } from '@/constant'

export default createContainer(Component, state => ({
  user: state.user,
  is_login: state.user.is_login,

  // TODO: we need a comparator for access level GE/LE
  is_admin: (state.user.is_admin || state.user.role === USER_ROLE.COUNCIL),
}), () => {
})
