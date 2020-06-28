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
  },
  txid: {
    type: String,
    default: ''
  },
  status: {
    type: String,
    emnu: _.values(constant.CVOTE_CHAIN_STATUS),
    default: constant.CVOTE_CHAIN_STATUS.UNCHAIN
  },
  signature: { data: String, message: String },
  reasonHash: {
    type: String,
  },
  reasonCreatedAt: {
    type: Date
  }
}

export const CVoteHistorySchema = {
  ...CVoteResultSchema,
  createdAt: {
    type: Date,
    required: true,
    default: Date.now
  }
}

const withdrawalHistorySchema = {
  message: String,
  messageHash: String,
  milestoneKey: String,
  signature: String,
  createdAt: Date,
  review: {
    reason: String,
    reasonHash: String,
    opinion: String,
    createdAt: Date
  }
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
    enum: _.values(constant.CONTENT_TYPE)
  },
  type: {
    type: String,
    enum: _.values(constant.CVOTE_TYPE)
  },
  abstract: {
    type: String
  },
  goal: {
    type: String
  },
  motivation: {
    type: String
  },
  relevance: {
    type: String
  },
  budgetAmount: {
    type: Number
  },
  elaAddress: {
    type: String
  },
  budget: {
    type: Schema.Types.Mixed
  },
  plan: {
    type: Schema.Types.Mixed
  },
  // name of proposer
  proposedBy: {
    type: String,
    required: true
  },
  proposer: {
    type: Schema.Types.ObjectId,
    ref: 'users',
    required: true
  },
  motionId: {
    type: String
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
  // council member
  createdBy: { type: Schema.Types.ObjectId, ref: 'users' },

  published: {
    type: Boolean,
    default: false
  },
  proposedAt: {
    type: Date
    // default: Date.now,
  },
  status: {
    type: String,
    enum: _.values(constant.CVOTE_STATUS)
  },
  subscribers: [SubscriberSchema],
  notified: {
    type: Boolean,
    default: false
  },
  reference: {
    type: Schema.Types.ObjectId,
    ref: 'suggestion'
  },
  referenceElip: {
    type: Schema.Types.ObjectId,
    ref: 'elip'
  },

  // from ELIP
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
  specifications: {
    type: String
  },
  rationale: {
    type: String
  },
  backwardCompatibility: {
    type: String
  },
  referenceImplementation: {
    type: String
  },
  copyright: {
    type: String
  },
  payment: {
    type: String
  },
  proposalHash: {
    type: String,
    required: true,
    unique: true
  },
  draftHash: {
    type: String,
    required: true,
    unique: true
  },
  ownerPublicKey: String,
  rejectAmount: {
    type: String
  },
  rejectThroughAmount: {
    type: String
  },
  withdrawalHistory: [withdrawalHistorySchema],
  old: Boolean // mark an old proposal
}
