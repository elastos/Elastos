import BaseRedux from '@/model/BaseRedux'

class ReleaseRedux extends BaseRedux {
  defineTypes() {
    return ['release']
  }

  defineDefaultState() {
    return {
      active_release: null,

      loading: false,

      all_releases: [],
      all_releases_total: 0,
    }
  }
}

export default new ReleaseRedux()
