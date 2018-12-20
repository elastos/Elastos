import {createContainer} from '@/util'
import Component from './Component'
import CouncilService from '@/service/CouncilService'
import {USER_ROLE} from '@/constant'
// import SubmissionService from '@/service/SubmissionService'
// import _ from 'lodash'

import {SUBMISSION_TYPE, USER_EMPOWER_TYPE} from '@/constant'

export default createContainer(Component, (state) => {
    return {
        user: state.user,
        is_login: state.user.is_login,

        // TODO: we need a comparator for access level GE/LE
        is_admin: (state.user.role === USER_ROLE.ADMIN || state.user.role === USER_ROLE.COUNCIL),

        council: state.council
    }
}, () => {

    const councilService = new CouncilService()

    return {
        async changeTab(tabKey) {
            return councilService.changeTab(tabKey)
        }
    }
})
