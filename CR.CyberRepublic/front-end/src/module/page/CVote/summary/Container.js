import { createContainer } from '@/util'
import Component from './Component'
import _ from 'lodash'

const mapState = state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary,
  isCouncil: state.user.is_council,
  canManage: state.user.is_secretary || state.user.is_council,
  latestStatus: _.get(_.last([...state.cvoteSummary.all_public, ...state.cvoteSummary.all_private]), 'status'),
})

const mapDispatch = () => {
}

export default createContainer(Component, mapState, mapDispatch)
