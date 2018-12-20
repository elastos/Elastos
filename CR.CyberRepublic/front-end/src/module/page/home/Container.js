import {createContainer} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'
import _ from 'lodash'

import {TASK_CATEGORY, TASK_TYPE} from '@/constant'

export default createContainer(Component, (state, ownProps) => {

    let taskState = state.task

    taskState.dev_tasks = []
    taskState.social_tasks = []

    // we have two sections, one for dev tasks/projects and the other for events,
    // each can only have 3 items max
    if (!_.isArray(state.task.all_tasks)) {

        _.values(state.task.all_tasks).map((task) => {
            if (task.type === TASK_TYPE.EVENT) {
                taskState.social_tasks.length < 3 && taskState.social_tasks.push(task)

            } else if (task.category === TASK_CATEGORY.DEVELOPER && (
                task.type === TASK_TYPE.TASK || task.type === TASK_TYPE.PROJECT
            )){
                taskState.dev_tasks.length < 3 && taskState.dev_tasks.push(task)
            }
        })
    }

    return {
        taskState,
        language: state.language
    }

}, () => {

    const taskService = new TaskService()

    return {
        async getTasks () {
            return taskService.index()
        },

        resetTasks () {
            return taskService.resetAllTasks()
        }
    }
})
