import {
  createContainer,
} from '@/util'
import Component from './Component'
import SuggestionService from '@/service/SuggestionService'
import CouncilService from '@/service/CouncilService'

const mapState = state => ({
  loading: state.suggestion.loading,
  councilMembers: state.council.council_members,
})

const mapDispatch = () => {
  const suggestionService = new SuggestionService()
  const councilService = new CouncilService()
  return {
    async create(param) {
      return suggestionService.create(param)
    },
    async update(param) {
      return suggestionService.update(param)
    },
    async getCouncilMembers() {
      return councilService.getCouncilMembers()
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
