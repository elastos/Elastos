import { Schema } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../../constant'
import { SubscriberSchema } from './SubscriberSchema'


export const CVoteResultSchema = {
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

export const CVoteHistorySchema = {
  ...CVoteResultSchema,
  createdAt: {
    type: Date,
    required: true,
    default: Date.now,
  },
}

export const CVote = {
  title: {
    type: String,
    required: true
  },
  vid: {
    type: Number,
    required: true
  },
  contentType: {
    type: String,
    enum: _.values(constant.CONTENT_TYPE),
  },
  abstract: {
    type: String,
  },
  goal: {
    type: String,
  },
  motivation: {
    type: String,
  },
  relevance: {
    type: String,
  },
  budget: {
    type: String,
  },
  plan: {
    type: String,
  },
  // tracking: {
  //   type: String,
  // },
  // summary: {
  //   type: String,
  // },
  // name of proposer
  proposedBy: {
    type: String,
    required: true
  },
  proposedByEmail: {
    type: String,
    required: true
  },
  motionId: {
    type: String,
  },
  notes: {
    type: String
  },
  voteHistory: [CVoteHistorySchema],
  voteResult: [CVoteResultSchema],
  vote_map: Object,
  avatar_map: Object,
  reason_map: Object,
  reason_zh_map: Object,
  createdBy: { type: Schema.Types.ObjectId, ref: 'users' },

  published: {
    type: Boolean,
    default: false,
  },
  proposedAt: {
    type: Date,
    // default: Date.now,
  },
  status: {
    type: String,
    enum: _.values(constant.CVOTE_STATUS)
  },
  subscribers: [SubscriberSchema],
  notified: {
    type: Boolean,
    default: false,
  },
  reference: {
    type: Schema.Types.ObjectId,
    ref: 'suggestion',
  }
}
