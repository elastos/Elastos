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
      council_members: [],
      council_members_loading: false,
    }
  }
}

export default new CouncilRedux()
