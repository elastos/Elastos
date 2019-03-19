import Base from '../Base'
import translate from './translate'

export default Base.setRouter([
  {
    path: '/translate',
    router: translate,
    method: 'post',
  },
])
