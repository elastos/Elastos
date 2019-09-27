import { Schema } from 'mongoose'
import { constant } from '../../constant'
import * as _ from 'lodash'

const SuggestionCore = {
  title: {
    type: String
  },

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
  budget: {
    type: String
  },
  plan: {
    type: String
  }
}


export const SuggestionEditHistory = {
  ...SuggestionCore,
  suggestion: {
    type: Schema.Types.ObjectId,
    ref: 'suggestion',
    required: true
  }
}
