import Base from '../Base'

import get from './get'
// import create from './create'
import list from './list'
import update from './update'
import archive from './archive'
import comment from './comment'
import subscribe from './subscribe'
import unsubscribe from './unsubscribe'

export default Base.setRouter([
    // {
    //     path : '/create',
    //     router : create,
    //     method : 'post'
    // },
    {
        path : '/list',
        router : list,
        method : 'get'
    },
    {
        path : '/:submissionId',
        router : get,
        method : 'get'
    },
    {
        path : '/campaign/:campaign',
        router : get,
        method : 'get'
    },
    {
        path : '/:submissionId',
        router : archive,
        method : 'delete'
    },
    {
        path : '/:submissionId',
        router : update,
        method : 'put'
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
