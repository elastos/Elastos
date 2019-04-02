import Base from '../Base'

import get from './get'
import create from './create'
import list from './list'
import update from './update'
import addCandidate from './add_candidate'
import removeCandidate from './remove_candidate'
import acceptCandidate from './accept_candidate'
import rejectCandidate from './reject_candidate'
import withdrawCandidate from './withdraw_candidate'
import markComplete from './mark_complete'
import markVisited from './mark_visited'
import comment from './comment'
import subscribe from './subscribe'
import unsubscribe from './unsubscribe'
import register from './register'
import deregister from './deregister'
import updateCandidate from './update_candidate'

export default Base.setRouter([

    {
        path : '/create',
        router : create,
        method : 'post'
    },
    {
        path : '/list',
        router : list,
        method : 'get'
    },
    {
        path : '/:taskId',
        router : get,
        method : 'get'
    },
    {
        path : '/:taskId',
        router : update,
        method : 'put'
    },
    {
        path : '/addCandidate',
        router : addCandidate,
        method : 'post'
    },
    {
        path : '/removeCandidate',
        router : removeCandidate,
        method : 'post'
    },
    {
        path : '/register',
        router : register,
        method : 'post'
    },
    {
        path : '/deregister',
        router : deregister,
        method : 'post'
    },
    {
        path : '/acceptCandidate',
        router : acceptCandidate,
        method : 'post'
    },
    {
        path : '/rejectCandidate',
        router : rejectCandidate,
        method : 'post'
    },
    {
        path : '/withdrawCandidate',
        router : withdrawCandidate,
        method : 'post'
    },
    {
        path: '/updateCandidate',
        router: updateCandidate,
        method: 'post'
    },
    {
        path: '/markTaskComplete',
        router: markComplete,
        method: 'post'
    },
    {
        path: '/markVisited',
        router: markVisited,
        method: 'post'
    },
    {
        path : '/:id/comment',
        router : comment,
        method : 'post'
    },
    {
        path : '/:id/subscribe',
        router : subscribe,
        method : 'post'
    },
    {
        path : '/:id/unsubscribe',
        router : unsubscribe,
        method : 'post'
    }
])
