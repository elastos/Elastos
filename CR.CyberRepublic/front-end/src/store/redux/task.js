import BaseRedux from '@/model/BaseRedux'

export const FETCH_TASK_BEGIN = 'FETCH_TASK_BEGIN'
export const FETCH_TASK_SUCCESS = 'FETCH_TASK_SUCCESS'
export const FETCH_TASK_FAILURE = 'FETCH_TASK_FAILURE'

class TaskRedux extends BaseRedux {
  defineTypes() {
    return ['task']
  }

  defineDefaultState() {
    return {
      active_task: null,

      loading: false,

      create_form: {
        task_type: 'EVENT',
      },

      all_tasks: [],
      all_tasks_total: 0,

      // if we select a detail
      detail: {},

      filter: {},
    }
  }
}

export default new TaskRedux()
