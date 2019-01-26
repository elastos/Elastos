db.task_candidates.aggregate([
  {
    $lookup: {
      from: 'tasks',
      localField: 'task',
      foreignField: '_id',
      as: 'task'
    }
  },
  {
    $lookup: {
      from: 'users',
      localField: 'user',
      foreignField: '_id',
      as: 'user__user'
    }
  },
  {
    $unwind: {
      path: '$task'
    }
  },
  {
    $unwind: {
      path: '$user__user'
    }
  },
  {
    $match: {
      'task.category': 'CR100'
    }
  },
  {
    $project: {
      'task.category': 1,
      'task.type': 1,
      'task.domain': 1,
      'task.name': 1,
      'task.description': 1,
      'applyMsg': 1,
      'attachment': 1,
      'attachmentFilename': 1,
      'user__user.profile.firstName': 1,
      'user__user.profile.lastName': 1,
      'user__user.profile.country': 1,
      'user__user.email': 1,
    }
  }
]).toArray()
