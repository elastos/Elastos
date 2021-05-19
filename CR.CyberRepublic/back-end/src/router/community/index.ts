import Base from '../Base'

import add_member from './add_member'
import remove_member from './remove_member'
import list_member from './list_member'
import get from './get'
import create from './create'
import update from './update'
import get_with_country from './get_with_country'
import get_child from './get_child'
import delete_community from './delete'
import get_all from './get_all'
import get_communities_with_user from './get_communities_with_user'

export default Base.setRouter([
    {
        path : '/',
        router : get,
        method : 'get'
    },
    {
        path : '/all',
        router : get_all,
        method : 'get'
    },
    {
        path : '/:communityId',
        router : get,
        method : 'get'
    },
    {
        path : '/:communityId',
        router : delete_community,
        method : 'delete'
    },
    {
        path : '/create',
        router : create,
        method : 'post'
    },
    {
        path : '/:communityId/members',
        router : list_member,
        method : 'get'
    },
    {
        path : '/:communityId/:userId',
        router : add_member,
        method : 'post'
    },
    {
        path : '/:communityId/:userId',
        router : remove_member,
        method : 'delete'
    },
    {
        path : '/update',
        router : update,
        method : 'put'
    },
    {
        path : '/country/:countryName',
        router : get_with_country,
        method : 'get'
    },
    {
        path : '/parent/:communityId',
        router : get_child,
        method : 'get'
    },
    {
        path : '/:userId/communities',
        router : get_communities_with_user,
        method : 'get'
    }
])
