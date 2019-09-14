import { createContainer } from '@/util'
import Component from './Component'
import CVoteSummaryService from '@/service/CVoteSummaryService'
import CVoteService from '@/service/CVoteService'

const mapState = state => ({
  isSecretary: state.user.is_secretary,
  privateList: state.cvoteSummary.all_private,
})

const mapDispatch = () => {
  const service = new CVoteSummaryService()
  const serviceCVote = new CVoteService()
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
    async getCVoteData(param) {
      return serviceCVote.getData(param)
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
