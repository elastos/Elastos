import { Schema } from 'mongoose'
import { constant } from '../../constant'
import * as _ from 'lodash'

export const Permission = {
  name: String,
  desc: String,
  resourceType: String,
  url: String,
  httpMethod: {
    type: String,
    enum: ['get', 'post', 'put', 'delete', 'patch']
  },
}

export const PermissionRole = {
  resourceType: String,
  role: {
    type: String,
    enum: _.values(constant.USER_ROLE)
  },
  permissionId: { type: Schema.Types.ObjectId, ref: 'permission' },
  isAllowed: Boolean,
  url: String,
  httpMethod: {
    type: String,
    enum: ['get', 'post', 'put', 'delete', 'patch']
  }
}
