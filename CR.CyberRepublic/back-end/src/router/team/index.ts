import Base from '../Base'
// import create from './create'
import update from './update'
import add_candidate from './add_candidate'
import action from './action'
import get from './get'
import list from './list'
import comment from './comment'


export default Base.setRouter([
    // {
    //     path : '/create',
    //     router : create,
    //     method : 'post'
    // },
    {
        path : '/update',
        router : update,
        method : 'post'
    },
    {
        path : '/addCandidate',
        router : add_candidate,
        method : 'post'
    },
    {
        path : '/action/:action',
        router : action,
        method : 'post'
    },
    {
        path : '/list',
        router : list,
        method : 'get'
    },
    {
        path : '/:teamId',
        router : get,
        method : 'get'
    },
    {
        path : '/:id/comment',
        router : comment,
        method : 'post'
    },
])
