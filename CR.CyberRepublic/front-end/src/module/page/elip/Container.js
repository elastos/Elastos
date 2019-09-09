import { createContainer } from '@/util'
import Component from './Component'

export default createContainer(
  Component,
  state => ({
    user: state.user,
    is_login: state.user.is_login
  }),
  () => {}
)
