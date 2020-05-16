import Base from '../Base'

import update_proposal from './update_proposal'
import proposal from './proposal'
import council from './council'
import candidates from './candidates'

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
    {
        path: '/council/list',
        router: council,
        method: 'get'
    },
    {
        path: '/candidates/list',
        router: candidates,
        method: 'get'
    }
])
