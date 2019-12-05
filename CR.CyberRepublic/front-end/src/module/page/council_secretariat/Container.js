import { createContainer } from '@/util'
import Component from './Component'
import CouncilService from '@/service/CouncilService'
import CandidateService from '@/service/CandidateService'

export default createContainer(Component, state => ({
  council: state.council,
}), () => {
  const councilService = new CouncilService()
  const service = new CandidateService()

  return {
    async changeTab(tabKey) {
      return councilService.changeTab(tabKey)
    },
    async listData(param) {
      return service.listData(param)
    }
  }
})
