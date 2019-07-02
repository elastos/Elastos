import { Schema } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../../constant'
import { CommentSchema } from './CommentSchema'

export const CVote_Summary = {
  content: {
    type: String,
    required: true
  },
  status: {
    type: String,
    enum: _.values(constant.CVOTE_SUMMARY_STATUS)
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
