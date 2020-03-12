import { Schema } from 'mongoose'

export const Did = {
  userId: {
    type: Schema.Types.ObjectId,
    ref: 'users'
  },
  did: String,
  expirationDate: Date,
  number: {
    type: String, // uuid string
    required: true,
    unique: true
  }
}
