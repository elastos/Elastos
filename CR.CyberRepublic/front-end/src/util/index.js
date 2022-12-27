import { connect } from 'react-redux'
import _ from 'lodash'
import { withRouter } from 'react-router'
import { api_request, upload_file, wallet_request } from './request'
import { loginRedirectWithQuery } from './login'
import * as permissions from './permissions'
import * as url from './url'
import * as editor from './editor'
import * as logger from './logger'

/**
 * Helper for React-Redux connect
 *
 * @param component
 * @param mapState - map state to props
 * @param mapDispatch - map dispatch to props
 */
export const createContainer = (
  component,
  mapState,
  mapDispatch = _.noop()
) => {
  const tmp_mapState = (state, ownProps) => {
    const s = {
      lang: state.language.language
    }

    return _.merge(s, mapState(state, ownProps))
  }
  return withRouter(connect(tmp_mapState, mapDispatch)(component))
}

export const constant = (moduleName, detailArray) => {
  const result = {}
  _.each(detailArray, detail => {
    result[detail] = `${moduleName}/${detail}`
  })

  return result
}

export const wordCounter = data => {
  const pattern = /[a-zA-Z0-9_\u0392-\u03c9]+|[\u4E00-\u9FFF\u3400-\u4dbf\uf900-\ufaff\u3040-\u309f\uac00-\ud7af]+/g
  const m = data.match(pattern)
  let count = 0
  if (m === null) {
    return count
  }
  for (let i = 0; i < m.length; i++) {
    if (m[i].charCodeAt(0) >= 0x4e00) {
      count += m[i].length
    } else {
      count += 1
    }
  }
  return count
}

export {
  api_request,
  upload_file,
  wallet_request,
  loginRedirectWithQuery,
  permissions,
  url,
  editor,
  logger
}
