import { createContainer } from '@/util'
import Component from './Component'
import ElipService from '@/service/ElipService'
import ElipReviewService from '@/service/ElipReviewService'

const mapState = state => ({
  user: state.user,
  currentUserId: state.user.current_user_id,
  isLogin: state.user.is_login,
  isSecretary: state.user.is_secretary
})

const mapDispatch = () => {
  const service = new ElipService()
  const reviewService = new ElipReviewService()
  return {
    async getData(param) {
      return service.getData(param)
    },
    async review(param) {
      return reviewService.create(param)
    }
  }
}

export default createContainer(Component, mapState, mapDispatch)
