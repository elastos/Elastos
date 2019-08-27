import { Schema } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../../constant'
import { CommentSchema } from './CommentSchema'

export const Elip = {
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
  description: {
    type: String
  },
  createdBy: {
    type: Schema.Types.ObjectId,
    ref: 'users',
    required: true
  },
  status: {
    type: String,
    enum: _.values(constant.ELIP_STATUS)
  },
  comments: [[CommentSchema]]
}
