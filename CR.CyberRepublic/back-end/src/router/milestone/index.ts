import Base from '../Base'
import update from './update'

export default Base.setRouter([
  {
    path: '/:id/milestones/:milestoneKey',
    router: update,
    method: 'post'
  }
])
