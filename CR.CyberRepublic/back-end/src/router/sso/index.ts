import Base from '../Base'
import sso from './sso'

export default Base.setRouter([{
    path: '/login',
    router: sso,
    method: 'get'
}])