import BaseRedux from '@/model/BaseRedux'

export const FETCH_SUGGESTION_BEGIN = 'FETCH_SUGGESTION_BEGIN'
export const FETCH_SUGGESTION_SUCCESS = 'FETCH_SUGGESTION_SUCCESS'
export const FETCH_SUGGESTION_FAILURE = 'FETCH_SUGGESTION_FAILURE'

class SuggestionRedux extends BaseRedux {
  defineTypes() {
    return ['suggestion']
  }

  defineDefaultState() {
    return {
      active_suggestion: null,

      loading: false,

      create_form: {
      },

      all_suggestions: [],
      all_suggestions_total: 0,

      my_suggestions_loading: false,
      my_suggestions: [],
      my_suggestions_total: 0,

      // if we select a detail
      detail: {},
      sortBy: null,
      filter: {},
    };
  }
}

export default new SuggestionRedux()
