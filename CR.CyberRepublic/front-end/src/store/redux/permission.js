import BaseRedux from '@/model/BaseRedux'

class PermissionRedux extends BaseRedux {
  defineTypes() {
    return ['permission']
  }

  defineDefaultState() {
    return {
      active_permission: null,

      loading: false,

      all_permissions: [],
      all_permissions_total: 0,
    }
  }
}

export default new PermissionRedux()
