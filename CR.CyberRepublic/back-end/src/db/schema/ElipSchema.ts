import { Schema } from 'mongoose'
import * as _ from 'lodash'
import { constant } from '../../constant'
import { CommentSchema } from './CommentSchema'

export const Elip = {
  title: {
    type: String,
    required: true
  },
  vid: {
    type: Number
  },
  elipType: {
    type: String,
    enum: _.values(constant.ELIP_TYPE)
  },
  contentType: {
    type: String,
    enum: _.values(constant.CONTENT_TYPE)
  },
  createdBy: {
    type: Schema.Types.ObjectId,
    ref: 'users',
    required: true
  },
  reference: {
    type: Schema.Types.ObjectId,
    ref: 'cvote'
  },
  discussionsTo: {
    type: String
  },
  requires: {
    type: String
  },
  replaces: {
    type: String
  },
  supersededBy: {
    type: String
  },
  abstract: {
    type: String
  },
  specifications: {
    type: String
  },
  motivation: {
    type: String
  },
  rationale: {
    type: String
  },
  backwardCompatibility: {
    type: String
  },
  referenceImplementation: {
    type: String
  },
  copyright: {
    type: String
  },
  status: {
    type: String,
    enum: _.values(constant.ELIP_STATUS)
  },
  comments: [[CommentSchema]]
}
