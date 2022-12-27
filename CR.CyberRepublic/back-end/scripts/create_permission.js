// MAKE SURE YOU RUN BUILD BEFORE THIS

const PERMISSIONS = [
  // Proposal
  {
    name: 'create',
    resourceType: 'CVote',
    url: '/cvote/create',
    httpMethod: 'post',
    desc: 'Add Proposal',
  },
  {
    name: 'read',
    resourceType: 'CVote',
    url: '/cvote/get/:id',
    httpMethod: 'get',
    desc: 'View Proposal',
  },
  {
    name: 'update',
    resourceType: 'CVote',
    url: '/cvote/update',
    httpMethod: 'post',
    desc: 'Modify Proposal',
  },
  {
    name: 'delete',
    resourceType: 'CVote',
    url: '/cvote/:id/delete',
    httpMethod: 'delete',
    desc: 'Delete Proposal',
  },
  {
    name: 'updateVote',
    resourceType: 'CVote',
    url: '/cvote/vote',
    httpMethod: 'post',
    desc: 'Vote/Object Proposal',
  },
  {
    name: 'createNote',
    resourceType: 'CVote',
    url: '/cvote/update_notes',
    httpMethod: 'post',
    desc: 'Notes on Proposal (Notes from Seretary)',
  },
  {
    name: 'readNote',
    resourceType: 'CVote',
    url: '/cvote/:id',
    httpMethod: 'get',
    desc: 'View notes on proposal',
  },
  {
    name: 'deleteComment',
    resourceType: 'CVote',
    url: '/cvote/:id/comment/delete',
    httpMethod: 'delete',
    desc: 'Delete Comment',
  },

  // Suggestion
  {
    name: 'create',
    resourceType: 'Suggestion',
    url: '/suggestion/create',
    httpMethod: 'post',
    desc: 'Add Suggestion',
  },
  {
    name: 'read',
    resourceType: 'Suggestion',
    url: '/suggestion/:id/show',
    httpMethod: 'get',
    desc: 'View Suggestion',
  },
  {
    name: 'update',
    resourceType: 'Suggestion',
    url: '/suggestion/:id/update',
    httpMethod: 'put',
    desc: 'Modify Suggestion',
  },
  {
    name: 'delete',
    resourceType: 'Suggestion',
    url: '/suggestion/:id/delete',
    httpMethod: 'delete',
    desc: 'Delete Suggestion',
  },
  {
    name: 'like',
    resourceType: 'Suggestion',
    url: '/suggestion/:id/like',
    httpMethod: 'put',
    desc: 'Like',
  },
  {
    name: 'dislike',
    resourceType: 'Suggestion',
    url: '/suggestion/:id/dislike',
    httpMethod: 'put',
    desc: 'Dislike',
  },
  {
    name: 'reportabuse',
    resourceType: 'Suggestion',
    url: '/suggestion/:id/reportabuse',
    httpMethod: 'put',
    desc: 'Report Abuse',
  },
  {
    name: 'comment',
    resourceType: 'Suggestion',
    url: '/suggestion/:id/comment',
    httpMethod: 'post',
    desc: 'Comment',
  },
  {
    name: 'deleteComment',
    resourceType: 'Suggestion',
    url: '/suggestion/:id/comment/delete',
    httpMethod: 'delete',
    desc: 'Delete Comment',
  },

  // User
  // {
  //   name: 'updateRoleAndProfile',
  //   resourceType: 'User',
  //   url: '/user/updateRoleAndProfile',
  //   httpMethod: 'put',
  //   desc: 'Role Set',
  // },
  {
    name: 'update',
    resourceType: 'User',
    url: '/user/:id',
    httpMethod: 'put',
    desc: 'Role Set(Update User)',
  },
  {
    name: 'delete',
    resourceType: 'User',
    url: '/user/:id/delete',
    httpMethod: 'delete',
    desc: 'Delete User',
  },

  // Permission
  {
    name: 'createPermission',
    resourceType: 'Permission',
    url: '/permission/create',
    httpMethod: 'post',
    desc: 'Create Permission',
  },
  {
    name: 'listPermission',
    resourceType: 'Permission',
    url: '/permission/list',
    httpMethod: 'get',
    desc: 'List Permission',
  },
  {
    name: 'viewPermission',
    resourceType: 'Permission',
    url: '/permission/:id/show',
    httpMethod: 'get',
    desc: 'View Permission',
  },
  {
    name: 'updatePermission',
    resourceType: 'Permission',
    url: '/permission/:id/update',
    httpMethod: 'post',
    desc: 'Update Permission',
  },
  {
    name: 'deletePermission',
    resourceType: 'Permission',
    url: '/permission/:id/delete',
    httpMethod: 'post',
    desc: 'Delete Permission',
  },

  // PermissionRole
  {
    name: 'createPermissionRole',
    resourceType: 'PermissionRole',
    url: '/permissionRole/create',
    httpMethod: 'post',
    desc: 'Create PermissionRole',
  },
  {
    name: 'listPermissionRole',
    resourceType: 'PermissionRole',
    url: '/permissionRole/list',
    httpMethod: 'get',
    desc: 'List PermissionRole',
  },
  {
    name: 'viewPermissionRole',
    resourceType: 'PermissionRole',
    url: '/permissionRole/:id/show',
    httpMethod: 'get',
    desc: 'View PermissionRole',
  },
  {
    name: 'updatePermissionRole',
    resourceType: 'PermissionRole',
    url: '/permissionRole/:id/update',
    httpMethod: 'post',
    desc: 'Update PermissionRole',
  },
  {
    name: 'deletePermissionRole',
    resourceType: 'PermissionRole',
    url: '/permissionRole/:id/delete',
    httpMethod: 'post',
    desc: 'Delete PermissionRole',
  }
]



const _ = require('lodash')
require('../dist/src/config');

(async () => {
  const db = await require('../dist/src/db').default
  const DB = await db.create()
  const db_permission = DB.getModel('Permission')

  try {
    for (const perm of PERMISSIONS) {
      db_permission.update({ name: perm.name }, { $set: {
        ...perm
      }}, { upsert: true })
    }
  } catch (err) {
      console.error(err)
  }

  process.exit(1)
})()