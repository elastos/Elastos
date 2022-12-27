import {
  createContainer,
} from '@/util'
import {
  USER_ROLE,
} from '@/constant'
import Service from '@/service/PermissionService'
import PermissionRoleService from '@/service/PermissionRoleService'
import Component from './Component'

const mapState = (state) => {
  const currentUserId = state.user.current_user_id
  const isAdmin = state.user.role === USER_ROLE.ADMIN
  const {
    loading,
    all_permissions: dataList,
    all_permissions_total: total,
  } = state.permission
  const {
    loading: loadingForRole,
    all_permission_roles: dataListForRole,
    all_permission_roles_total: totalForRole,
  } = state.permissionRole
  const permissionState = {
    ...state.permission,
    dataList,
    total,
    loading,
    currentUserId,
    isAdmin,
    loadingForRole,
    dataListForRole,
    totalForRole,
  }

  return permissionState
}

const mapDispatch = () => {
  const service = new Service()
  const permissionRoleService = new PermissionRoleService()

  return {
    async create(param) {
      return service.create(param)
    },
    async update(param) {
      return service.update(param)
    },
    async getList(query) {
      return service.list({
        ...query,
      })
    },
    resetAll() {
      return service.resetAll()
    },
    // Permission Role
    async getListForRole(query) {
      return permissionRoleService.list({
        ...query,
      })
    },
    async updateForRole(query) {
      return permissionRoleService.update({
        ...query,
      })
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
