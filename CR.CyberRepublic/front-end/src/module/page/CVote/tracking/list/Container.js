import { createContainer } from '@/util'
import Component from './Component'
import CVoteTrackingService from '@/service/CVoteTrackingService'

const mapState = state => ({
  isSecretary: state.user.is_secretary,
  privateList: state.cvoteTracking.all_private,
})

const mapDispatch = () => {
  const service = new CVoteTrackingService()
  return {
    async create(param) {
      return service.create(param)
    },
    async reject(param) {
      return service.reject(param)
    },
    async approve(param) {
      return service.approve(param)
    },
    async listData(param) {
      return service.listData(param)
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
