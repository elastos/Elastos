import { createContainer } from '@/util'
import Component from './Component'
import CandidateService from '@/service/CandidateService'

const mapState = state => ({})

const mapDispatch = () => {
  const service = new CandidateService()
  return {
    async listData(param) {
      return service.listData(param)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
