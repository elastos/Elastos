import {createContainer} from '@/util'
// import UserService from '@/service/UserService'
import Component from './Component'
import {TASK_TYPE, TASK_CATEGORY} from '@/constant'
import SubmissionService from '@/service/SubmissionService'
import TeamService from '@/service/TeamService'
import _ from 'lodash'

import {SUBMISSION_TYPE, USER_EMPOWER_TYPE} from '@/constant'

export default createContainer(Component, (state) => {
    const myCircles = _.values(state.user.circles)

    return {
        ...state.team,
        myCircles,
        user: state.user,
        is_login: state.user.is_login,
        currentUserId: state.user.current_user_id,
        language: state.language.language
    }
}, () => {
    // const userService = new UserService()
    const submissionService = new SubmissionService();
    const teamService = new TeamService()

    return {
        async getTeams(query) {
            return teamService.index(query)
        },

        async loadAllCircles() {
            return teamService.loadAllCircles()
        },

        resetAllTeams() {
            return teamService.resetAllTeams()
        }
    }
})
