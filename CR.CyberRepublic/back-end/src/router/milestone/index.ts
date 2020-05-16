import Base from '../Base'
import update from './update'

export default Base.setRouter([
  {
    path: '/:id/milestones/:stage',
    router: update,
    method: 'post'
  }
])
