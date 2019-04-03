import BaseRedux from '@/model/BaseRedux'

export const FETCH_SUBMISSION_BEGIN = 'FETCH_SUBMISSION_BEGIN'
export const FETCH_SUBMISSION_SUCCESS = 'FETCH_SUBMISSION_SUCCESS'
export const FETCH_SUBMISSION_FAILURE = 'FETCH_SUBMISSION_FAILURE'

class SubmissionRedux extends BaseRedux {
  defineTypes() {
    return ['submission']
  }

  defineDefaultState() {
    return {
      active_submission: null,

      loading: false,

      create_form: {
        submission_type: 'BUG',
      },

      all_submissions: [],

      // if we select a detail
      detail: {},

      // filter - persistent filter
      filter: {},
    }
  }
}

export default new SubmissionRedux()
