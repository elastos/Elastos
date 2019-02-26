import Base from '../Base'
import create from './create'
import createMany from './createMany'
import list from './list'
import show from './show'
import del from './delete'
import update from './update'

export default Base.setRouter([
  {
    path: '/create',
    router: create,
    method: 'post',
  },
  {
    path: '/createMany',
    router: createMany,
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
  {
    path: '/:id/update',
    router: update,
    method: 'post',
  },
  {
    path: '/:id/delete',
    router: del,
    method: 'post',
  },
])
