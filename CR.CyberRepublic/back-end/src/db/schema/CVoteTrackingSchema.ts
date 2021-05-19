import { Schema } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../../constant'

const CommentSchema = {
  createdBy: {
      type: Schema.Types.ObjectId,
      ref: 'users',
  },
  content: {
      type: String,
  },
  createdAt: {
    type: Date,
    default: Date.now,
  }
}

export const CVote_Tracking = {
  content: {
    type: String,
    required: true
  },
  status: {
    type: String,
    enum: _.values(constant.CVOTE_TRACKING_STATUS)
  },
  comment: CommentSchema,
  proposalId: {
    type: Schema.Types.ObjectId,
    ref: 'cvote'
  },
  createdBy: {
    type: Schema.Types.ObjectId,
    ref: 'users'
  },
}
