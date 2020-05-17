import Base from '../Base'
import update from './update'
import signatureCallback from './owner_signature_callback'

export default Base.setRouter([
  {
    path: '/:id/milestones/:milestoneKey',
    router: update,
    method: 'post'
  },
  {
    path: '/milestones/signature-callback',
    router: signatureCallback,
    method: 'post'
  }
])
