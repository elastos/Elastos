db.submissions.aggregate([
  {
    $lookup: {
      from: 'users',
      localField: 'createdBy',
      foreignField: '_id',
      as: 'createdBy_user'
    }
  },
  {
    $match: {
      type: 'EMPOWER_35'
    }
  },
  {
    $unwind: {
      path: '$createdBy_user'
    }
  },
  {
    $project: {
      comments: 1,
      type: 1,
      campaign: 1,
      title: 1,
      reason: 1,
      suitedReason: 1,
      attachment: 1,
      attachmentType: 1,
      attachmentFilename: 1,
      createdAt: 1,
      'createdBy_user.email': 1,
      'createdBy_user.profile.firstName': 1,
      'createdBy_user.profile.lastName': 1,
      'createdBy_user.profile.country': 1
    }
  }
]).toArray()
