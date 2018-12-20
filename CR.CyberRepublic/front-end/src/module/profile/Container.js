import {createContainer} from "@/util"
import Component from './Component'
import UserService from '@/service/UserService'
import {message} from 'antd'
import {TASK_STATUS} from '@/constant'

export default createContainer(Component, (state) => {
    return {
        is_admin: state.user.is_admin,
        is_login: state.user.is_login,
        currentUserId: state.user.current_user_id
    }
}, () => {
    const userService = new UserService()

    return {
        async getCurrentUser() {
            try {
                const rs = await userService.getCurrentUser()
            } catch (err) {
                message.error(err.message)
            }
        },

        async updateUser(userId, data) {
            await userService.update(userId, data)
        }
    }
})
