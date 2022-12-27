import { Schema } from 'mongoose'
import { constant } from '../../constant'
import * as _ from 'lodash'
import { SuggestionCore } from './SuggestionCoreSchema'

export const SuggestionEditHistory = {
  ...SuggestionCore,
  version: {
    type: Number
  },
  suggestion: {
    type: Schema.Types.ObjectId,
    ref: 'suggestion',
    required: true
  }
}

