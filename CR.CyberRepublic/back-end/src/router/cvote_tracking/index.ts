import Base from '../Base'

import create from './create'
import update from './update'
import list from './list'
import list_public from './list_public'
import approve from './approve'
import reject from './reject'

export default Base.setRouter([
  {
    path: '/create',
    router: create,
    method: 'post'
  },
  {
    path: '/update',
    router: update,
    method: 'post'
  },
  {
    path: '/list_public',
    router: list_public,
    method: 'get'
  },
  {
    path: '/list',
    router: list,
    method: 'get'
  },
  {
    path: '/approve',
    router: approve,
    method: 'post'
  },
  {
    path: '/reject',
    router: reject,
    method: 'post'
  }
])
