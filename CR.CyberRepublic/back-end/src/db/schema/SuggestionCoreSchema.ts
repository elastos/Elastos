import { Schema } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../../constant'

// plan data structure
const planSchema = {
  milestone: [
    {
      date: String,
      version: String, // goal
      milestoneKey: String
    }
  ],
  teamInfo: [
    {
      member: String,
      role: String,
      responsibility: String,
      info: String
    }
  ]
}
// budget data structure
const budgetSchema = {
  type: {
    type: String,
    enum: _.values(constant.SUGGESTION_BUDGET_TYPE)
  },
  amount: String,
  reasons: String,
  criteria: String,
  milestoneKey: String
}

export const SuggestionCore = {
  title: {
    type: String
  },
  descUpdatedAt: Date,

  // old fields
  shortDesc: {
    type: String,
    maxLength: 255
  },
  desc: {
    type: String
  },
  benefits: {
    type: String
  },
  funding: {
    type: Number
  },
  timeline: {
    type: Date
  },
  link: [String],
  coverImg: String,

  // new fields
  type: {
    type: String,
    enum: _.values(constant.SUGGESTION_TYPE)
  },
  abstract: {
    type: String
  },
  goal: {
    type: String
  },
  motivation: {
    type: String
  },
  relevance: {
    type: String
  },
  budgetAmount: {
    type: Number
  },
  elaAddress: {
    type: String
  },
  budget: {
    type: Schema.Types.Mixed
  },
  plan: {
    type: Schema.Types.Mixed
  }
}
