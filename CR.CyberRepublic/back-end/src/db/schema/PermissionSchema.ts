import { Schema } from 'mongoose'
import { constant } from '../../constant'
import * as _ from 'lodash'

export const PROPOSAL_PERMISSIONS = [
  {
    name: 'create',
    resourceType: 'Proposal',
    url: '/proposal/create',
    httpMethod: 'post',
    desc: 'Add Proposal',
  },
  {
    name: 'read',
    resourceType: 'Proposal',
    url: '/proposal/create',
    httpMethod: 'get',
    desc: 'View Proposal',
  },
  {
    name: 'update',
    resourceType: 'Proposal',
    url: '/proposal/update',
    httpMethod: 'post',
    desc: 'Edit Proposal',
  },
  {
    name: 'delete',
    resourceType: 'Proposal',
    url: '/proposal/:id/delete',
    httpMethod: 'post',
    desc: 'Delete Proposal',
  },
  {
    name: 'updateVote',
    resourceType: 'Proposal',
    url: '/proposal/update_notes',
    httpMethod: 'post',
    desc: 'Vote/Object/Abstain Proposal',
  },
  {
    name: 'createNote',
    resourceType: 'Proposal',
    url: '/proposal/create',
    httpMethod: 'post',
    desc: 'Notes on Proposal (Notes from Seretary)',
  },
  {
    name: 'readNote',
    resourceType: 'Proposal',
    url: '/proposal/:id',
    httpMethod: 'get',
    desc: 'View notes on proposal',
  },
]

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

export const url = {
  resourceType: 'Proposal',
  url: '/suggestion(/*)?',
  permissionId: '123',
  httpMethod: 'get',
  role: 'MEMBER',
  isAllowed: true,
  // allow: ['MEMBER', 'ADMIN', 'COUNCIL', 'SECRETARY'],
  // permissions: [
  //   {role: 'MEMBER', httpMethod: 'get', isAllowed: true},
  //   {role: 'ADMIN', httpMethod: 'get', isAllowed: true},
  //   {role: 'MEMBER', httpMethod: 'post', isAllowed: false},
  //   {role: 'ADMIN', httpMethod: 'post', isAllowed: false},
  // ],
}

export const PermissionUrl = {
  // action: {
  //   type: String,
  //   enum: ['allow', 'deny']
  // },
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
  },
  // permissions: {
  // }
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
  },
  // permissions: [
  //   {
  //   },
  // ],
  // deny: [{ type: Schema.Types.ObjectId, ref: 'permission', default: [] }],
  // allow: [{ type: Schema.Types.ObjectId, ref: 'permission', default: [] }],
}


// proposal access control
export const roles = [
  {
    role: 'COUNCIL',
    resourceType: 'Proposal',
    permissionId: '123',
    isAllowed: true
    // allow: ['create', 'read', 'update', 'delete', 'updateVote', 'createNote', 'readNote'], // from db [ObjectId], ====> ref <=====
    // deny: [],
  },
  {
    role: 'SECRETARY',
    resourceType: 'Proposal',
    allow: ['create', 'read', 'update', 'delete', 'createNote', 'readNote'], // from db
    deny: [],
  },
  {
    role: 'ADMIN',
    resourceType: 'Proposal',
    allow: ['read', 'delete', 'readNote'], // from db
    deny: [],
  },
  {
    role: 'CUSTOM',
    resourceType: 'Proposal',
    allow: ['read', 'delete', 'readNote'], // from db
    deny: [],
  },
  {
    role: 'MEMBER',
    resourceType: 'Proposal',
    allow: ['read', 'readNote'], // from db
    deny: [],
    // permissions: [
    //   {permissionName: 'read', httpMethod: 'get', isAllowed: true},
    //   {permissionName: 'readNote', httpMethod: 'get', isAllowed: true},
    // ],

  },
]
