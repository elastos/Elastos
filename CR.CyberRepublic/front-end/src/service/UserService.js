import BaseService from '../model/BaseService'
import _ from 'lodash'
import {USER_ROLE} from '@/constant'
import {api_request} from '@/util';

export default class extends BaseService {

    async login(username, password, persist) {
        const userRedux = this.store.getRedux('user')

        // call API /login
        const res = await api_request({
            path: '/api/user/login',
            method: 'get',
            data: {
                username,
                password
            }
        });

        this.dispatch(userRedux.actions.login_form_reset())

        this.dispatch(userRedux.actions.is_login_update(true))

        if ([USER_ROLE.ADMIN, USER_ROLE.COUNCIL].includes(res.user.role)) {
            this.dispatch(userRedux.actions.is_admin_update(true))
        }else{
            this.dispatch(userRedux.actions.is_admin_update(false))
        }
        if ([USER_ROLE.LEADER].includes(res.user.role)) {
            this.dispatch(userRedux.actions.is_leader_update(true))
        }else {
            this.dispatch(userRedux.actions.is_leader_update(false))
        }

        this.dispatch(userRedux.actions.email_update(res.user.email))
        this.dispatch(userRedux.actions.username_update(res.user.username))
        this.dispatch(userRedux.actions.profile_update(res.user.profile))
        this.dispatch(userRedux.actions.role_update(res.user.role))
        this.dispatch(userRedux.actions.circles_update(_.values(res.user.circles)))
        this.dispatch(userRedux.actions.subscribers_update(_.values(res.user.subscribers)))
        this.dispatch(userRedux.actions.current_user_id_update(res.user._id))
        sessionStorage.setItem('api-token', res['api-token']);

        if (persist) {
            localStorage.setItem('api-token', res['api-token'])
        } else {
            localStorage.removeItem('api-token')
        }

        return {
            success: true,
            is_admin: res.user.role === USER_ROLE.ADMIN
        }
    }

    /**
     * Check if username already exists
     *
     * @param username
     * @returns {Promise<void>}
     */
    async checkUsername(username) {
        return await api_request({
            path: '/api/user/check-username',
            method: 'post',
            data: {
                username
            }
        });
    }

    async register(username, password, profile) {

        const res = await api_request({
            path : '/api/user/register',
            method : 'post',
            data : Object.assign(profile, {
                username,
                password
            })
        });

        return this.login(username, password)
    }

    async forgotPassword(email) {

        return api_request({
            path : '/api/user/forgot-password',
            method : 'post',
            data : {
                email
            }
        })
    }

    async resetPassword(resetToken, password) {

        return api_request({
            path : '/api/user/reset-password',
            method : 'post',
            data : {
                resetToken,
                password
            }
        })
    }

    async getCurrentUser() {
        const userRedux = this.store.getRedux('user')

        const data = await api_request({
            path: '/api/user/current_user'
        })

        this.dispatch(userRedux.actions.is_login_update(true));
        if ([USER_ROLE.LEADER].includes(data.role)) {
            this.dispatch(userRedux.actions.is_leader_update(true))
        } else {
            this.dispatch(userRedux.actions.is_leader_update(false))
        }
        if ([USER_ROLE.ADMIN, USER_ROLE.COUNCIL].includes(data.role)) {
            this.dispatch(userRedux.actions.is_admin_update(true))
        } else {
            this.dispatch(userRedux.actions.is_admin_update(false))
        }
        this.dispatch(userRedux.actions.email_update(data.email))
        this.dispatch(userRedux.actions.username_update(data.username))
        this.dispatch(userRedux.actions.profile_reset())
        this.dispatch(userRedux.actions.profile_update(data.profile))
        this.dispatch(userRedux.actions.role_update(data.role))
        this.dispatch(userRedux.actions.current_user_id_update(data._id))

        this.dispatch(userRedux.actions.circles_update(_.values(data.circles)))
        this.dispatch(userRedux.actions.loading_update(false))

        return data
    }

    // restrictive getter - public profile should never return email / private info
    async getMember(userId, options = {}) {
        let path = `/api/user/public/${userId}`
        const memberRedux = this.store.getRedux('member')
        await this.dispatch(memberRedux.actions.loading_update(true))

        if (options.admin) {
            path += '?admin=true'
        }

        const result = await api_request({
            path: path,
            method: 'get'
        })

        await this.dispatch(memberRedux.actions.detail_update(result))
        await this.dispatch(memberRedux.actions.loading_update(false))

        return result
    }

    resetMemberDetail() {
        const memberRedux = this.store.getRedux('member')
        this.dispatch(memberRedux.actions.detail_reset())
    }

    async update(userId, doc) {

        const memberRedux = this.store.getRedux('member')

        await this.dispatch(memberRedux.actions.loading_update(true))

        const result = await api_request({
            path: `/api/user/${userId}`,
            method: 'put',
            data: doc
        })

        await this.dispatch(memberRedux.actions.detail_update(result))
        await this.dispatch(memberRedux.actions.loading_update(false))

        return result
    }

    async logout() {
        const userRedux = this.store.getRedux('user')
        const tasksRedux = this.store.getRedux('task')

        return new Promise((resolve) => {
            this.dispatch(userRedux.actions.is_login_update(false))
            this.dispatch(userRedux.actions.profile_reset())

            this.dispatch(userRedux.actions.is_admin_reset())
            this.dispatch(userRedux.actions.is_leader_reset())

            this.dispatch(userRedux.actions.email_reset())
            this.dispatch(userRedux.actions.username_reset())
            this.dispatch(userRedux.actions.profile_reset())
            this.dispatch(userRedux.actions.role_reset())
            this.dispatch(userRedux.actions.circles_reset())
            this.dispatch(userRedux.actions.current_user_id_reset())

            this.dispatch(tasksRedux.actions.all_tasks_reset())
            sessionStorage.clear()
            localStorage.removeItem('api-token', '')
            resolve(true)
        })
    }

    async getByIds(ids) {
        const result = await api_request({
            path : `/api/user/${ids}/users`,
            method : 'get',
        });

        return result
    }

    async getAll(query) {
        const memberRedux = this.store.getRedux('member')
        await this.dispatch(memberRedux.actions.users_loading_update(true))

        const path = '/api/user/list'
        this.abortFetch(path)

        let result
        try {
            result = await api_request({
                path,
                method : 'get',
                data: query,
                signal: this.getAbortSignal(path)
            });

            await this.dispatch(memberRedux.actions.users_update(result.list))
            await this.dispatch(memberRedux.actions.users_total_update(result.total))
            await this.dispatch(memberRedux.actions.users_loading_update(false))
        } catch (e) {
            // Do nothing
        }

        return result
    }

    async sendEmail(fromUserId, toUserId, formData) {
        return await api_request({
            path: '/api/user/send-email',
            method: 'post',
            data: {
                fromUserId,
                toUserId,
                ...formData
            }
        })
    }

    async sendRegistrationCode(email, code) {
        return await api_request({
            path: '/api/user/send-code',
            method: 'post',
            data: {
                email,
                code // TODO dont send this in clear text
            }
        })
    }

    async sendConfirmationEmail(email) {
        return await api_request({
            path: '/api/user/send-confirm',
            method: 'post',
            data: {
                email
            }
        })
    }

    async checkEmail(email) {
        return await api_request({
            path: '/api/user/check-email',
            method: 'post',
            data: {
                email
            }
        })
    }
}
