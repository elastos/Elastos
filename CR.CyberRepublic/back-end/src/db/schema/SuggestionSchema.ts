import { Schema } from 'mongoose'
import { CommentSchema } from './CommentSchema'
import { SubscriberSchema } from './SubscriberSchema'
import { constant } from '../../constant'
import * as _ from 'lodash'

const SuggestionCore = {
    title: {
        type: String,
        required: true,
        // minlength: 1,
        // maxLength: 150,
    },
    desc: {
        type: String,
        required: true,
        // minlength: 1,
        // maxLength: 10000,
    },
    link: String,
}

export const Suggestion = {
    ...SuggestionCore,
    editHistory: [{
        ...SuggestionCore,
        updatedAt: {
            type: Date,
            default: Date.now,
        }
    }],
    likes: {
        type: [Schema.Types.ObjectId],
        default: [],
    },
    likesNum: {
        type: Number,
        default: 0,
    },
    dislikes: {
        type: [Schema.Types.ObjectId],
        default: [],
    },
    dislikesNum: {
        type: Number,
        default: 0,
    },
    viewsNum: {
        type: Number,
        default: 0
    },
    activeness: {
        type: Number,
        default: 0
    },
    comments: [[CommentSchema]],
    commentsNum: {
        type: Number,
        default: 0,
    },
    createdBy: {
        type: Schema.Types.ObjectId,
        ref: 'users',
        required: true
    },
    // constans.SUGGESTION_STATUS: ACTIVE, ABUSED, ARCHIVED. abuse will also be archived
    status: {
        type: String,
        uppercase: true,
        enum: _.values(constant.SUGGESTION_STATUS),
        default: constant.SUGGESTION_STATUS.ACTIVE,
    },
    // constant.SUGGESTION_ABUSED_STATUS: REPORTED, HANDLED
    abusedStatus: {
        type: String,
        uppercase: true,
        enum: _.values(constant.SUGGESTION_ABUSED_STATUS)
    },
    subscribers: [SubscriberSchema],
    // subscribersNum: {
    //     type: Number,
    //     default: 0,
    // },
}
