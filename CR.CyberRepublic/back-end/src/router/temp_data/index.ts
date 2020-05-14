import Base from '../Base'

import update_proposal from './update_proposal'
import proposal from './proposal'

export default Base.setRouter([
    {
        path: '/update',
        router: update_proposal,
        method: 'post'
    },
    {
        path: '/:id',
        router: proposal,
        method: 'get'
    },
])
