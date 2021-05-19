import {combineReducers} from 'redux'
import {routerReducer} from 'react-router-redux'

import cvote from './redux/cvote'
import cvoteTracking from './redux/cvoteTracking'
import cvoteSummary from './redux/cvoteSummary'
import task from './redux/task'
import user from './redux/user'
import team from './redux/team'
import community from './redux/community'
import member from './redux/member'
import submission from './redux/submission'
import suggestion from './redux/suggestion'
import council from './redux/council'
import language from './redux/language'
import permission from './redux/permission'
import permissionRole from './redux/permissionRole'
import release from './redux/release'
import elip from './redux/elip'

const default_state = {
  init: false,
}

const appReducer = (state = default_state) => {
  // switch (action.type) {}

  return state
}

export default combineReducers({
  app: appReducer,
  router: routerReducer,
  cvote: cvote.getReducer(),
  cvoteTracking: cvoteTracking.getReducer(),
  cvoteSummary: cvoteSummary.getReducer(),
  task: task.getReducer(),
  user: user.getReducer(),
  team: team.getReducer(),
  community: community.getReducer(),
  member: member.getReducer(),
  submission: submission.getReducer(),
  suggestion: suggestion.getReducer(),
  council: council.getReducer(),
  language: language.getReducer(),
  permission: permission.getReducer(),
  permissionRole: permissionRole.getReducer(),
  release: release.getReducer(),
  elip: elip.getReducer()
})
