import BaseRedux from '@/model/BaseRedux'

class CVoteSummaryRedux extends BaseRedux {
  defineTypes() {
    return ['cvote']
  }

  defineDefaultState() {
    return {
      loading: false,
      data: undefined,
      all_cotes: [],
      all_cotes_total: 0,
      filters: {},
    }
  }
}

export default new CVoteSummaryRedux()
