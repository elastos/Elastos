import Base from '../Base'
import create from './create'
import list from './list'
import show from './show'
import like from './like'
import dislike from './dislike'
import reportabuse from './reportabuse'
import subscribe from './subscribe'
import unsubscribe from './unsubscribe'
import comment from './comment'
import abuse from './abuse'
import archive from './archive'
import del from './delete'

export default Base.setRouter([
  {
    path: '/create',
    router: create,
    method: 'post',
  },
  {
    path: '/list',
    router: list,
    method: 'get',
  },
  {
    path: '/:id/show',
    router: show,
    method: 'get',
  },
  // {
  //   path: '/:id/update',
  //   router: update,
  //   method: 'post',
  // },
  {
    path: '/:id/like',
    router: like,
    method: 'post',
  },
  {
    path: '/:id/dislike',
    router: dislike,
    method: 'post',
  },
  {
    path: '/:id/reportabuse',
    router: reportabuse,
    method: 'post',
  },
  {
    path : '/:id/comment',
    router : comment,
    method : 'post'
  },
  {
    path : '/:id/subscribe',
    router : subscribe,
    method : 'post'
  },
  {
    path : '/:id/unsubscribe',
    router : unsubscribe,
    method : 'post'
  },
  {
    path: '/:id/abuse',
    router: abuse,
    method: 'post',
  },
  {
    path: '/:id/archive',
    router: archive,
    method: 'post',
  },
  {
    path: '/:id/delete',
    router: del,
    method: 'post',
  },
])
