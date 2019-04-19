import BaseRedux from '@/model/BaseRedux'

class CouncilRedux extends BaseRedux {
  defineTypes() {
    return ['council']
  }

  defineDefaultState() {
    return {
      loading: false,
      tab: 'COUNCIL',
      filter: {},
    }
  }
}

export default new CouncilRedux()
