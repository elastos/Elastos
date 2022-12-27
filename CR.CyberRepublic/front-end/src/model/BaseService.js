/* global AbortController */
import store from '@/store'
import 'abortcontroller-polyfill/dist/abortcontroller-polyfill-only'

export default class {
  constructor() {
    this.store = store
    this.path = store.history
    this.abortControllers = []

    this.init()
  }

  init() {}

  dispatch(action) {
    return this.store.dispatch(action)
  }

  getAbortSignal(path) {
    this.abortControllers[path] = this.abortControllers[path] || new AbortController()
    return this.abortControllers[path].signal
  }

  abortFetch(path) {
    const controller = this.abortControllers[path]

    if (controller) {
      controller.abort()
      delete this.abortControllers[path]
    }
  }
}
