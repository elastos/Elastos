import {createContainer, goPath} from "@/util"
import Component from './Component'
import UserService from '@/service/UserService'
import {message} from 'antd'
import I18N from '@/I18N'

message.config({
    top: 100
})

export default createContainer(Component, (state) => {
    return {
        ...state.user.login_form,
        language: state.language
    }
}, () => {
    const userService = new UserService()

    return {
        async login(username, password, persist) {
            try {
                const rs = await userService.login(username.trim(), password, persist)

                if (rs) {
                    message.success(I18N.get('login.success'))

                    const loginRedirect = sessionStorage.getItem('loginRedirect')
                    if (loginRedirect) {
                        console.log('login redirect')
                        this.history.push(loginRedirect)
                        sessionStorage.setItem('loggedIn', '1')
                        sessionStorage.setItem('loginDirect', null)
                    } else {
                        this.history.push('/developer')
                    }
                    return true
                }

            } catch (err) {
                message.error(err.message)
                return false
            }
        }
    }
})
