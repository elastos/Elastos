import { createContainer } from '@/util'
import Component from './Component'

const mapState = state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login
})

const mapDispatch = () => {}

export default createContainer(Component, mapState, mapDispatch)
