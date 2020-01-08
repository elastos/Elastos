import { createContainer } from '@/util'
import Component from './Component'
import ElipService from '@/service/ElipService'
import ElipReviewService from '@/service/ElipReviewService'

const mapState = state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isAdmin: state.user.is_admin,
  isSecretary: state.user.is_secretary,
  isCouncil: state.user.is_council,
  data: state.elip.detail,
  reviews: state.elip.reviews
})

const mapDispatch = () => {
  const service = new ElipService()
  const reviewService = new ElipReviewService()
  return {
    async getData(param) {
      return service.getData(param)
    },
    async resetData() {
      return service.resetData()
    },
    async review(param) {
      return reviewService.create(param)
    },
    async vote(param) {
      return param
    },
    async updateStatus(param) {
      return service.update(param)
    },
    async deleteData(param) {
      return service.deleteData(param)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
