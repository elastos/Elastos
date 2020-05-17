import Base from '../Base'
import update from './update'
import signatureCallback from './owner_signature_callback'
import checkSignature from './check_signature'

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
  },
  {
    path: '/:id/milestones/signature',
    router: checkSignature,
    method: 'post'
  }
])
