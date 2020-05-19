import Base from '../Base'
import applyPayment from './apply_payment'
import signatureCallback from './owner_signature_callback'
import checkSignature from './check_signature'
import secSignatureCallback from './sec_signature_callback'
import review from './review'

export default Base.setRouter([
  {
    path: '/:id/milestones/:milestoneKey',
    router: applyPayment,
    method: 'post'
  },
  {
    path: '/milestones/signature-callback',
    router: signatureCallback,
    method: 'post'
  },
  {
    path: '/milestones/signature',
    router: checkSignature,
    method: 'post'
  },
  {
    path: '/:id/milestones/:milestoneKey/review',
    router: review,
    method: 'post'
  },
  {
    path: '/milestones/sec-signature-callback',
    router: secSignatureCallback,
    method: 'post'
  }
])
