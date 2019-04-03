import Base from '../Base'
import ping from './ping'

export default Base.setRouter([
    {
        path : '/',
        router : ping,
        method : 'get'
    }
])
