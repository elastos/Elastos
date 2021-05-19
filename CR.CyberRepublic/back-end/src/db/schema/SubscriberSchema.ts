import { Schema } from 'mongoose'

export const SubscriberSchema = {
  user: {
    type: Schema.Types.ObjectId,
    ref: 'users',
    required: true
  },
  lastSeen: {
    type: Date,
    required: false
  }
}
