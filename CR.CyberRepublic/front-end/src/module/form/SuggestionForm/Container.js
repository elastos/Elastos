import {
  createContainer,
} from '@/util'
import Component from './Component'
import SuggestionService from '@/service/SuggestionService'
import GoogleService from '@/service/GoogleService'

const mapState = state => ({
  loading: state.suggestion.loading,
})

const mapDispatch = () => {
  const suggestionService = new SuggestionService();
  const googleService = new GoogleService();
  return {
    async create(param) {
      return suggestionService.create(param)
    },
    async gTranslate(param) {
      const res = await googleService.translate(param)
      return res
    },
  };
}

export default createContainer(Component, mapState, mapDispatch)
