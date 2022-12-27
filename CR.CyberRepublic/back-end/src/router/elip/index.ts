import Base from '../Base'
import create from './create'
import detail from './detail'
import update from './update'
import list from './list'
import comment from './comment'
import comment_remove from './comment_remove'
import remove from './remove'

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
    path: '/list',
    router: list,
    method: 'get'
  },
  {
    path: '/:id/comment',
    router: comment,
    method: 'post'
  },
  {
    path : '/:id/comment_remove',
    router : comment_remove,
    method : 'post'
  }
])
