import Base from '../Base'

import get from './get'
import login from './login'
import register from './register'
import update from './update'
import updateRole from './updateRole'
import current_user from './current_user'
import send_email from './send_email'
import send_reg_email from './send_reg_email'
import send_confirm_email from './send_confirm_email'
import check_email from './check_email'
import change_password from './change_password'
import forgot_password from './forgot_password'
import reset_password from './reset_password'
import list_users from './list_users'
import getCouncilMembers from './getCouncilMembers'
import comment from './comment'
import subscribe from './subscribe'
import unsubscribe from './unsubscribe'
import logout from './logout'
import elaUrl from './ela_url'
import loginElaUrl from './login_ela_url'
import didCallbackEla from './did_callback_ela'
import loginCallbackEla from './login_callback_ela'
import getDid from './get_did'
import checkElaAuth from './check_ela_auth'

export default Base.setRouter([
    {
        path: '/logout',
        router: logout,
        method: 'get'
    },
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
        path : '/:userId/updateRole',
        router : updateRole,
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
        path : '/getCouncilMembers',
        router : getCouncilMembers,
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
    },
    {
        path: '/ela-url',
        router: elaUrl,
        method: 'get'
    },
    {
        path: '/did-callback-ela',
        router: didCallbackEla,
        method: 'post'
    },
    {
        path: '/did',
        router: getDid,
        method: 'get'
    },
    {
        path: '/login-ela-url',
        router: loginElaUrl,
        method: 'get'
    },
    {
        path: '/login-callback-ela',
        router: loginCallbackEla,
        method: 'post'
    },
    {
        path: '/check-ela-auth',
        router: checkElaAuth,
        method: 'post'
    }
])
