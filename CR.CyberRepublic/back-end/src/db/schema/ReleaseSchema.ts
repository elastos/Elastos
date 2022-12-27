import { Schema } from 'mongoose'

export const Release = {
  title: String,
  desc: String,
  createdBy: {
    type: Schema.Types.ObjectId,
    ref: 'users',
    required: true
  },
}