import { createContainer } from '@/util'
import Component from './Component'
import CouncilService from '@/service/CouncilService'

export default createContainer(Component, state => ({
  council: state.council,
}), () => {
  const councilService = new CouncilService()

  return {
    async changeTab(tabKey) {
      return councilService.changeTab(tabKey)
    },
    async getCandidates(param) {
      return councilService.getCandidates(param)
    },
    async getCouncilsAndSecretariat() {
      return councilService.getCouncilsAndSecretariat()
    }
  }
})
