/*global AbortController*/
import store from '@/store';

export default class {
    constructor() {
        this.store = store
        this.path = store.history
        this.abortControllers = []

        this.init()
    }

    init(){
        let isLogout = !localStorage.getItem('api-token') && sessionStorage.getItem('api-token')
        if (isLogout) {
            this.path.go(0)
        }
    }

    dispatch(action){
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
};
