import Base from '../Base';

import get from './get';
import login from './login';
import register from './register';
import update from './update';
import current_user from './current_user';
import send_email from './send_email';
import send_reg_email from './send_reg_email';
import send_confirm_email from './send_confirm_email';
import check_email from './check_email';
import change_password from './change_password';
import forgot_password from './forgot_password';
import reset_password from './reset_password';
import list_users from './list_users';
import comment from './comment'
import subscribe from './subscribe'
import unsubscribe from './unsubscribe'

export default Base.setRouter([
    {
        path : '/login',
        router : login,
        method : 'get'
    },
    {
        path : '/register',
        router : register,
        method : 'post'
    },
    {
        path : '/:userId',
        router : update,
        method : 'put'
    },
    {
        path : '/public/:userId',
        router : get,
        method : 'get'
    },
    {
        path : '/send-email',
        router : send_email,
        method : 'post'
    },
    {
        path : '/send-code',
        router : send_reg_email,
        method : 'post'
    },
    {
        path: '/send-confirm',
        router: send_confirm_email,
        method: 'post'
    },
    {
        path: '/check-email',
        router: check_email,
        method: 'post'
    },
    {
        path : '/current_user',
        router : current_user,
        method : 'get'
    },
    {
        path : '/change_password',
        router : change_password,
        method : 'get'
    },
    {
        path : '/forgot-password',
        router : forgot_password,
        method : 'post'
    },
    {
        path : '/reset-password',
        router : reset_password,
        method : 'post'
    },
    {
        path : '/:userIds/users',
        router : list_users,
        method : 'get'
    },
    {
        path : '/list',
        router : list_users,
        method : 'get'
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
]);
