import { Schema } from 'mongoose'
import { constant } from '../../constant'
import * as _ from 'lodash'
import { Suggestion } from './SuggestionSchema'

export const SuggestionDraft = {
  ...Suggestion
}
