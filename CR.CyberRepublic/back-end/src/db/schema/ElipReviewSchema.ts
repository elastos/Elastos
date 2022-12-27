import { Schema } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../../constant'

export const Elip_Review = {
  comment: {
    type: String
  },
  status: {
    type: String,
    enum: _.values(constant.ELIP_REVIEW_STATUS)
  },
  elipId: {
    type: Schema.Types.ObjectId,
    ref: 'elip'
  },
  createdBy: {
    type: Schema.Types.ObjectId,
    ref: 'users'
  },
  createdAt: {
    type: Date,
    default: Date.now,
  }
}
