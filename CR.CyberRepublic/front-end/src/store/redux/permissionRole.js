import BaseRedux from '@/model/BaseRedux'

class PermissionRoleRedux extends BaseRedux {
  defineTypes() {
    return ['permission_role']
  }

  defineDefaultState() {
    return {
      active_permission_role: null,

      loading: false,

      all_permission_roles: [],
      all_permission_roles_total: 0,

      // if we select a detail
      detail: {},
    }
  }
}

export default new PermissionRoleRedux()
