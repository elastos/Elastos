import { Schema } from 'mongoose'

export const CommentSchema = {
  createdBy: {
    type: Schema.Types.ObjectId,
    ref: 'users',
    required: true
  },
  comment: {
    type: String,
    required: true
  },
  headline: String,
  createdAt: Date
}
