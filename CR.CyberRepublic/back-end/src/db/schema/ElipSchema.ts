import { Schema } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../../constant'
import { CommentSchema } from './CommentSchema'

export const ElipResultSchema = {
  votedBy: {
    type: Schema.Types.ObjectId,
    ref: 'users',
    required: true
  },
  value: {
    type: String,
    emun: _.values(constant.CVOTE_RESULT),
    default: constant.CVOTE_RESULT.UNDECIDED
  },
  reason: {
    type: String,
    default: ''
  }
}

export const ElipHistorySchema = {
  ...CVoteResultSchema,
  createdAt: {
    type: Date,
    required: true,
    default: Date.now
  }
}

export const Elip = {
  title: {
    type: String,
    required: true
  },
  vid: {
    type: Number,
    required: true
  },
  elipType: {
    type: String,
    enum: _.values(constant.ELIP_TYPE)
  },
  contentType: {
    type: String,
    enum: _.values(constant.CONTENT_TYPE)
  },
  createdBy: {
    type: Schema.Types.ObjectId,
    ref: 'users',
    required: true
  },
  discussionsTo: {
    type: String
  },
  requires: {
    type: String
  },
  replaces: {
    type: String
  },
  supersededBy: {
    type: String
  },
  // description = Abstract
  description: {
    type: String
  },
  specifications: {
    type: String
  },
  motivation: {
    type: String
  },
  rationale: {
    type: String
  },
  backwardCompatibility: {
    type: String
  },
  copyright: {
    type: String
  },
  status: {
    type: String,
    enum: _.values(constant.ELIP_STATUS)
  },
  voteHistory: [ElipHistorySchema],
  voteResult: [ElipResultSchema],
  comments: [[CommentSchema]]
}
