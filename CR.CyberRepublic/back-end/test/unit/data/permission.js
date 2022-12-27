
global.DB = Object.assign(global.DB, {

  PERMISSIONS: {
    // Release
    Release: {
      create: {
        name: 'create',
        resourceType: 'Release',
        url: '/release/create',
        httpMethod: 'post',
        desc: 'Add Release',
      },
      read: {
        name: 'read',
        resourceType: 'Release',
        url: '/release/:id/show',
        httpMethod: 'get',
        desc: 'View Release',
      },
      update: {
        name: 'update',
        resourceType: 'Release',
        url: '/release/:id/update',
        httpMethod: 'put',
        desc: 'Modify Release',
      },
      delete: {
        name: 'delete',
        resourceType: 'Release',
        url: '/release/:id/delete',
        httpMethod: 'delete',
        desc: 'Delete Release',
      },
    },
    // Proposal
    Proposal: {
      create: {
        name: 'create',
        resourceType: 'CVote',
        url: '/cvote/create',
        httpMethod: 'post',
        desc: 'Add Proposal',
      },
      read: {
        name: 'read',
        resourceType: 'CVote',
        url: '/cvote/get/:id',
        httpMethod: 'get',
        desc: 'View Proposal',
      },
      update: {
        name: 'update',
        resourceType: 'CVote',
        url: '/cvote/update',
        httpMethod: 'post',
        desc: 'Modify Proposal',
      },
      delete: {
        name: 'delete',
        resourceType: 'CVote',
        url: '/cvote/:id/delete',
        httpMethod: 'delete',
        desc: 'Delete Proposal',
      },
      updateVote: {
        name: 'updateVote',
        resourceType: 'CVote',
        url: '/cvote/vote',
        httpMethod: 'post',
        desc: 'Vote/Object Proposal',
      },
      createNote: {
        name: 'createNote',
        resourceType: 'CVote',
        url: '/cvote/update_notes',
        httpMethod: 'post',
        desc: 'Notes on Proposal (Notes from Seretary)',
      },
      readNote: {
        name: 'readNote',
        resourceType: 'CVote',
        url: '/cvote/:id',
        httpMethod: 'get',
        desc: 'View notes on proposal',
      },
      deleteComment: {
        name: 'deleteComment',
        resourceType: 'CVote',
        url: '/cvote/:id/comment/delete',
        httpMethod: 'delete',
        desc: 'Delete Comment',
      },
    },

    // Suggestion
    Suggestion: {
      create: {
        name: 'create',
        resourceType: 'Suggestion',
        url: '/suggestion/create',
        httpMethod: 'post',
        desc: 'Add Suggestion',
      },
      read: {
        name: 'read',
        resourceType: 'Suggestion',
        url: '/suggestion/:id/show',
        httpMethod: 'get',
        desc: 'View Suggestion',
      },
      update: {
        name: 'update',
        resourceType: 'Suggestion',
        url: '/suggestion/:id/update',
        httpMethod: 'put',
        desc: 'Modify Suggestion',
      },
      delete: {
        name: 'delete',
        resourceType: 'Suggestion',
        url: '/suggestion/:id/delete',
        httpMethod: 'delete',
        desc: 'Delete Suggestion',
      },
      like: {
        name: 'like',
        resourceType: 'Suggestion',
        url: '/suggestion/:id/like',
        httpMethod: 'put',
        desc: 'Like',
      },
      dislike: {
        name: 'dislike',
        resourceType: 'Suggestion',
        url: '/suggestion/:id/dislike',
        httpMethod: 'put',
        desc: 'Dislike',
      },
      reportabuse: {
        name: 'reportabuse',
        resourceType: 'Suggestion',
        url: '/suggestion/:id/reportabuse',
        httpMethod: 'put',
        desc: 'Report Abuse',
      },
      comment: {
        name: 'comment',
        resourceType: 'Suggestion',
        url: '/suggestion/:id/comment',
        httpMethod: 'post',
        desc: 'Comment',
      },
      deleteComment: {
        name: 'deleteComment',
        resourceType: 'Suggestion',
        url: '/suggestion/:id/comment/delete',
        httpMethod: 'delete',
        desc: 'Delete Comment',
      },
    },
    // User
    User: {
      updateRole: {
        name: 'updateRole',
        resourceType: 'User',
        url: '/user/:id/updateRole',
        httpMethod: 'put',
        desc: 'Role Set',
      },
      updateUser: {
        name: 'updateUser',
        resourceType: 'User',
        url: '/user/:id',
        httpMethod: 'put',
        desc: 'Update User',
      },
      deleteUser: {
        name: 'deleteUser',
        resourceType: 'User',
        url: '/user/:id/delete',
        httpMethod: 'delete',
        desc: 'Delete User',
      },
    },
  },
})
