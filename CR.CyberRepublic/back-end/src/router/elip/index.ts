import Base from '../Base'
import create from './create'
import detail from './detail'
import update from './update'
import list from './list'
import comment from './comment'
import vote from './vote'
import remove from './remove'
import propose from './propose'

export default Base.setRouter([
  {
    path: '/create',
    router: create,
    method: 'post'
  },
  {
    path: '/detail/:id',
    router: detail,
    method: 'get'
  },
  {
    path: '/update',
    router: update,
    method: 'post'
  },
  {
    path: '/:id/remove/',
    router: remove,
    method: 'get'
  },
  {
    path: '/:id/propose/',
    router: propose,
    method: 'get'
  },
  {
    path: '/vote',
    router: vote,
    method: 'post'
  },
  {
    path: '/list',
    router: list,
    method: 'get'
  },
  {
    path: '/:id/comment',
    router: comment,
    method: 'post'
  }
])
