import { createContainer } from '@/util'
import Component from './Component'
import _ from 'lodash'
import ElipService from '@/service/ElipService'

const mapState = state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary
})

const mapDispatch = () => {
  const service = new ElipService()
  return {
    async getData(param) {
      return service.getData(param)
    },
    async update(param) {
      return service.update(param)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
