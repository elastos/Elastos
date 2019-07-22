import BaseRedux from '@/model/BaseRedux'

class CVoteSummaryRedux extends BaseRedux {
  defineTypes() {
    return ['cvote_summary']
  }

  defineDefaultState() {
    return {
      loading: false,

      all_public: [],
      all_public_total: 0,

      all_private: [],
      all_private_total: 0,
    }
  }
}

export default new CVoteSummaryRedux()
