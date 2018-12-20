import {combineReducers} from 'redux'
import {routerReducer} from 'react-router-redux'

import task from './redux/task'
import user from './redux/user'
import team from './redux/team'
import community from './redux/community'
import member from './redux/member'
import submission from './redux/submission'
import council from './redux/council'
import language from './redux/language'

const default_state = {
    init: false
};

const appReducer = (state = default_state, action) => {
    switch (action.type) {

    }

    return state
}

export default combineReducers({
    app: appReducer,
    router: routerReducer,
    task: task.getReducer(),
    user: user.getReducer(),
    team: team.getReducer(),
    community: community.getReducer(),
    member: member.getReducer(),
    submission: submission.getReducer(),
    council: council.getReducer(),
    language: language.getReducer()
})
