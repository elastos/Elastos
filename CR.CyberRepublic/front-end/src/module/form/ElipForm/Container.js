import { createContainer } from '@/util'
import Component from './Component'
import ElipService from '@/service/ElipService'

const mapState = state => ({
  user: state.user,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary
})

const mapDispatch = () => {
  const service = new ElipService()
  return {
    async create(param) {
      return service.create(param)
    },
    async update(param) {
      return service.update(param)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
