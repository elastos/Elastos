import { Schema } from 'mongoose'
import { CommentSchema } from './CommentSchema'
import { SubscriberSchema } from './SubscriberSchema'
import { constant } from '../../constant'
import * as _ from 'lodash'

const SuggestionCore = {
  title: {
    type: String,
    // required: true,
    // minlength: 1,
    // maxLength: 150,
  },
  shortDesc: {
    type: String,
    maxLength: 255
  },
  desc: {
    type: String,
    // required: true,
    // minlength: 1,
    // maxLength: 10000,
  },
  benefits: {
    type: String,
    // required: true,
  },
  funding: {
    type: Number,
  },
  timeline: {
    type: Date,
  },
  link: [String],
}

const tag = {
  type: {
    type: String,
    enum: _.values(constant.SUGGESTION_TAG_TYPE),
    uppercase: true,
    required: true,
  },
  desc: String,
  createdBy: {
    type: Schema.Types.ObjectId,
    ref: 'users',
    required: true
  },
  createdAt: {
    type: Date,
    default: Date.now,
  },
}

export const Suggestion = {
  ...SuggestionCore,
  editHistory: [{
    ...SuggestionCore,
    updatedAt: {
      type: Date,
      default: Date.now,
    }
  }],
  likes: {
    type: [Schema.Types.ObjectId],
    default: [],
  },
  likesNum: {
    type: Number,
    default: 0,
  },
  dislikes: {
    type: [Schema.Types.ObjectId],
    default: [],
  },
  dislikesNum: {
    type: Number,
    default: 0,
  },
  viewsNum: {
    type: Number,
    default: 0
  },
  activeness: {
    type: Number,
    default: 0
  },
  comments: [[CommentSchema]],
  commentsNum: {
    type: Number,
    default: 0,
  },
  createdBy: {
    type: Schema.Types.ObjectId,
    ref: 'users',
    required: true
  },
  // constans.SUGGESTION_STATUS: ACTIVE, ABUSED, ARCHIVED. abuse will also be archived
  status: {
    type: String,
    uppercase: true,
    enum: _.values(constant.SUGGESTION_STATUS),
    default: constant.SUGGESTION_STATUS.ACTIVE,
  },
  // constant.SUGGESTION_ABUSED_STATUS: REPORTED, HANDLED
  abusedStatus: {
    type: String,
    uppercase: true,
    enum: _.values(constant.SUGGESTION_ABUSED_STATUS)
  },
  subscribers: [SubscriberSchema],
  // subscribersNum: {
  //     type: Number,
  //     default: 0,
  // },

  reference: [
    {
      type: Schema.Types.ObjectId,
      ref: 'cvote',
    }
  ],
  tags: [tag],
}
