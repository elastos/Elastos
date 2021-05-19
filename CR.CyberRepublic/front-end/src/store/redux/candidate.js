import BaseRedux from '@/model/BaseRedux'

class CandidateRedux extends BaseRedux {
  defineTypes() {
    return ['candidate']
  }

  defineDefaultState() {
    return {
      data: undefined,
      detail: {},
    }
  }
}

export default new CandidateRedux()
