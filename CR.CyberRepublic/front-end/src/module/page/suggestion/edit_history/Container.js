import _ from 'lodash'
import {
  createContainer,
} from '@/util'
import Component from './Component'
import SuggestionService from '@/service/SuggestionService'

const mapState = state => ({
  dataList: _.get(state.suggestion.detail, 'editHistory'),
  loading: _.get(state.suggestion.detail, 'loading'),
  detail: state.suggestion.detail,
})

const mapDispatch = () => {
  const service = new SuggestionService()

  return {
    async getDetail({
      id,
      incViewsNum,
    }) {
      return service.getDetail({
        id,
        incViewsNum,
      })
    },
    resetDetail() {
      return service.resetDetail()
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
