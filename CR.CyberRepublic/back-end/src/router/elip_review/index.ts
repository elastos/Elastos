import Base from '../Base'
import create from './create'

export default Base.setRouter([
  {
    path: '/create',
    router: create,
    method: 'post'
  }
])
