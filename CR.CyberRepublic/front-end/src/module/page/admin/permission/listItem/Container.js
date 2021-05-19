import {
  createContainer,
} from '@/util'
import PermissionRoleService from '@/service/PermissionRoleService'
import Component from './Component'

const mapState = () => {}

const mapDispatch = () => {
  const permissionRoleService = new PermissionRoleService()

  return {
    async updateForRole(query) {
      return permissionRoleService.update({
        ...query,
      })
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
