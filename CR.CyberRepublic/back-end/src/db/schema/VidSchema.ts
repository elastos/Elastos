import { Schema } from 'mongoose'

export const Vid = {
  tableName: {
    type: String,
    required: true
  },
  vid: {
    type: Number,
    required: true
  }
}

