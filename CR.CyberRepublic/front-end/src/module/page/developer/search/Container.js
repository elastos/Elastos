import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService';
import TeamService from '@/service/TeamService';
import _ from 'lodash'

export default createContainer(Component, (state) => {
    return {
        ...state
    }

}, () => {
    return {
    }
})
