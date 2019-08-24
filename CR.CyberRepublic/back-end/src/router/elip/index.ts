import Base from '../Base'
import create from './create'
import detail from './detail'

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
  }
])
