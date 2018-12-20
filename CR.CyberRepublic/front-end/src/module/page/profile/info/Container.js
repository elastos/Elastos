import {createContainer} from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import { message } from 'antd/lib/index'

export default createContainer(Component, (state) => {

    return {
        user: state.user
    }
})
