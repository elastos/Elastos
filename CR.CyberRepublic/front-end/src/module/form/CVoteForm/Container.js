import { createContainer } from '@/util'
import Component from './Component'
import CVoteService from '@/service/CVoteService'

const mapState = state => ({
  user: state.user,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary,
  isCouncil: state.user.is_council,
  canManage: state.user.is_secretary || state.user.is_council
})

const mapDispatch = () => {
  const service = new CVoteService()
  return {
    async createDraft(param) {
      return service.createDraft(param)
    },
    async updateDraft(param) {
      return service.updateDraft(param)
    },
    async createCVote(param) {
      return service.createCVote(param)
    },
    async updateCVote(param) {
      return service.updateCVote(param)
    },
    async finishCVote(param) {
      return service.finishCVote(param)
    },
    async updateNotes(param) {
      return service.updateNotes(param)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
