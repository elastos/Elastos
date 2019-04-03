import _ from 'lodash'
import { constant } from '../util'

export default class {
  constructor() {
    this.reducers = {}
    this.actions = {}
    this.types = this.buildTypes()
    this._reducer = this.buildReducer()
    this.buildActions()
  }

  getPathName() {
    const userDefineTypes = this.defineTypes()
    if (!_.isArray(userDefineTypes) || !_.isString(userDefineTypes[0])) {
      throw new Error(`invalid redux types definition : ${userDefineTypes}`)
    }

    return userDefineTypes[0]
  }

  buildTypes() {
    const userDefineTypes = this.defineTypes()
    if (!_.isArray(userDefineTypes) || !_.isString(userDefineTypes[0])) {
      throw new Error(`invalid redux types definition : ${userDefineTypes}`)
    }

    const pathName = userDefineTypes[0]
    const types = {}
    _.each(this.defineDefaultState(), (value, key) => {
      // this adds a [field]_update action
      const update_key = `${key}_update`
      types[update_key] = `${pathName}/${key}_update`
      this.reducers[update_key] = (state, param) => {
        return {
          ...state,
          [key]: _.isArray(param)
            ? param
            : _.isObject(param)
              ? _.merge({}, state[key], param || {})
              : param
        }
      }
      this.actions[update_key] = (param) => {
        return {
          type: update_key,
          param: {
            key: param,
            pathname: pathName
          }
        }
      }

      // this adds a [field]_reset action which resets
      // the field to the default state
      const reset_key = `${key}_reset`
      types[reset_key] = `${pathName}/${key}_reset`
      this.reducers[reset_key] = (state) => {
        return {
          ...state,
          [key]: this.defineDefaultState()[key]
        }
      }
      this.actions[reset_key] = () => {
        return {
          type: reset_key,
          param: {
            pathname: pathName
          }
        }
      }
    })

    return _.extend(types, constant(userDefineTypes[0], userDefineTypes[1] || []))
  }

  buildReducer() {
    const pathName = this.getPathName()
    _.extend(this.reducers, this.defineReducers())
    return (state = this.defineDefaultState(), action) => {
      const type = action.type
      if (!action.param || action.param.pathname === pathName) {
        if (this.types[type] && this.reducers[type]) {
          return this.reducers[type](state, _.has(action.param, 'key') ? action.param.key : action.param)
        }
      }

      return state
    }
  }

  buildActions() {
    const userDefineActions = this.defineActions()
    _.extend(this.actions, userDefineActions)
  }

  getReducer() {
    return this._reducer
  }

  defineDefaultState() {
    return {}
  }

  defineTypes() {
    return []
  }

  defineActions() {
    return {}
  }

  defineReducers() {
    return {}
  }
}
