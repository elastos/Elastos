import Base from '../Base'

import term from './term'

export default Base.setRouter([
    {
        path: '/term',
        router: term,
        method: 'get'
    }
])
