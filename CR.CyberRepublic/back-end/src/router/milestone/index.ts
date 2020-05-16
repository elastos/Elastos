import Base from '../Base'
import update from './update'

export default Base.setRouter([
  {
    path: '/milestones/:stage',
    router: update,
    method: 'post'
  }
])
