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
      page: 1,

      // if we select a detail
      detail: {},
      draft: {},
      sortBy: null,
      filters: {},

      // filter on list
      tags_included: {
        infoNeeded: false,
        underConsideration: false
      },

      // filter on list - by default we don't show any suggestions added to proposals
      // TODO: this gets tricky because we can have a suggestion referenced by multiple proposals
      // the proper handling is to only query the latest proposal and change this to struct to define which statuses we want
      // for now I will just use a boolean to mean any proposal referencing it with any status
      reference_status: false,
      edit_history: [],
    }
  }
}

export default new SuggestionRedux()
