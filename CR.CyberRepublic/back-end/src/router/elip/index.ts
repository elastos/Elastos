import Base from '../Base'
import create from './create'
import detail from './detail'
import update from './update'
import list from './list'

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
    path: '/list',
    router: list,
    method: 'get'
  },
])
