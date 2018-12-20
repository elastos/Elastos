import Base from '../Base';

import create from './create';
import list from './list';
import list_public from './list_public';
import get from './get';
import update from './update';
import finish from './finish';
import update_notes from './update_notes';

export default Base.setRouter([
    {
        path : '/create',
        router : create,
        method : 'post'
    },
    {
        path : '/list_public',
        router : list_public,
        method : 'get'
    },
    {
        path : '/list',
        router : list,
        method : 'get'
    },
    {
        path : '/get/:id',
        router : get,
        method : 'get'
    },
    {
        path : '/update',
        router : update,
        method : 'post'
    },
    {
        path : '/finish',
        router : finish,
        method : 'get'
    },
    {
        path : '/update_notes',
        router : update_notes,
        method : 'post'
    }
]);
