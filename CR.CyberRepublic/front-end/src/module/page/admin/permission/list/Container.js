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
  return {}
}

const mapDispatch = () => {
  return {}
}

export default createContainer(Component, mapState, mapDispatch)
