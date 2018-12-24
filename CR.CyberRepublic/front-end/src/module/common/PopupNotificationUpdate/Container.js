import {createContainer} from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import {message} from "antd";

export default createContainer(Component, (state) => {
    return {
        is_login: state.user.is_login,
        currentUserId: state.user.current_user_id,
        popup_update: state.user.popup_update,
        user: state.user
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

        async updateUserPopupUpdate(userId) {
            await userService.update(userId, {
                popupUpdate: true
            })
        }
    }
})
