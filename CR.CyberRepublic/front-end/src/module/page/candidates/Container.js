import { createContainer } from '@/util'
import Component from './Component'
import CouncilService from '@/service/CouncilService'

const mapState = state => ({})

const mapDispatch = () => {
  const service = new CouncilService()
  return {
    async listData(param) {
      return service.getCandidates(param)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
