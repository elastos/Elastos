import Base from '../Base'
import create from './create'
import update from './update'
import saveDraft from './saveDraft'
import list from './list'
import export2csv from './export2csv'
import show from './show'
import showDraft from './showDraft'
import addTag from './addTag'
import like from './like'
import dislike from './dislike'
import reportabuse from './reportabuse'
import subscribe from './subscribe'
import unsubscribe from './unsubscribe'
import comment from './comment'
import comment_update from './comment_update'
import comment_remove from './comment_remove'
import abuse from './abuse'
import archive from './archive'
import del from './delete'
import editHistories from './editHistories'
import revertVersion from './revertVersion'
import investigation from './investigation'
import advisory from './advisory'
import signatureUrl from './signature_url'
import signatureCallback from './signature_callback'
import checkSignature from './check_signature'
import getSuggestion from './getSuggestion'
import cmSignatureUrl from './cm_signature_url'

export default Base.setRouter([
  {
    path: '/create',
    router: create,
    method: 'post'
  },
  {
    path: '/list',
    router: list,
    method: 'get'
  },
  {
    path: '/export2csv',
    router: export2csv,
    method: 'get'
  },
  {
    path: '/:id/show',
    router: show,
    method: 'get'
  },
  {
    path: '/:id/showDraft',
    router: showDraft,
    method: 'get'
  },
  {
    path: '/:id/update',
    router: update,
    method: 'put'
  },
  {
    path: '/:id/saveDraft',
    router: saveDraft,
    method: 'put'
  },
  {
    path: '/:id/addtag',
    router: addTag,
    method: 'post'
  },
  {
    path: '/:id/like',
    router: like,
    method: 'post'
  },
  {
    path: '/:id/dislike',
    router: dislike,
    method: 'post'
  },
  {
    path: '/:id/reportabuse',
    router: reportabuse,
    method: 'post'
  },
  {
    path: '/:id/comment',
    router: comment,
    method: 'post'
  },
  {
    path: '/:id/comment_update',
    router: comment_update,
    method: 'post'
  },
  {
    path: '/:id/comment_remove',
    router: comment_remove,
    method: 'post'
  },
  {
    path: '/:id/subscribe',
    router: subscribe,
    method: 'post'
  },
  {
    path: '/:id/unsubscribe',
    router: unsubscribe,
    method: 'post'
  },
  {
    path: '/:id/abuse',
    router: abuse,
    method: 'post'
  },
  {
    path: '/:id/archive',
    router: archive,
    method: 'post'
  },
  {
    path: '/:id/delete',
    router: del,
    method: 'post'
  },
  {
    path: '/:id/editHistories',
    router: editHistories,
    method: 'get'
  },
  {
    path: '/:id/revertVersion',
    router: revertVersion,
    method: 'post'
  },
  {
    path: '/:id/investigation',
    router: investigation,
    method: 'post'
  },
  {
    path: '/:id/advisory',
    router: advisory,
    method: 'post'
  },
  {
    path: '/signature-url',
    router: signatureUrl,
    method: 'post'
  },
  {
    path: '/signature-callback',
    router: signatureCallback,
    method: 'post'
  },
  {
    path: '/signature',
    router: checkSignature,
    method: 'post'
  },
  {
    path: '/get_suggestion/:id',
    router: getSuggestion,
    method: 'get'
  },
  {
    path: '/cm-signature-url',
    router: cmSignatureUrl,
    method: 'post'
  }
])
