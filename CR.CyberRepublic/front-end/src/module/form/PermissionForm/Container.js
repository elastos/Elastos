import {
  createContainer,
} from '@/util'
import Component from './Component'
import PermissionService from '@/service/PermissionService'

const mapState = state => ({
})

const mapDispatch = () => {
  const permissionService = new PermissionService()
  return {
    async create(param) {
      return permissionService.create(param)
    },
    async update(param) {
      return permissionService.update(param)
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
