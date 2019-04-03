import {Schema} from 'mongoose'
import {CommentSchema} from './CommentSchema'
import {SubscriberSchema} from './SubscriberSchema'

const communityProps = {
    community : {
        type: Schema.Types.ObjectId,
        ref: 'community'
    },
    state : {
        type : String
    },
    city : {
        type : String
    }
}

export const Submission = {
    type : {
        type : String,
        required : true
    },
    campaign: {
        type: String
    },
    title : {
        type : String,
        required : true
    },
    description : {
        type : String
    },

    email: String,
    fullLegalName: String,

    passportNumber: String,
    passportUpload: String, // Anniversary form - Passport Number
    passportUploadType: String,
    passportFilename: String,

    occupation: String,
    education: String,
    location: String,

    audienceInfo: String,
    publicSpeakingExp: String,
    eventOrganizingExp: String,
    previousExp: String,

    isDeveloper: Boolean,

    devBackground: String,

    dob: Date,

    reason: String,
    suitedReason: String,

    attachment: String,
    attachmentType: String,
    attachmentFilename: String,

    createdBy : {
        type: Schema.Types.ObjectId,
        ref: 'users'
    },
    comments: [[CommentSchema]],
    subscribers: [SubscriberSchema],
    lastCommentSeenByOwner: Date,

    ...communityProps,

    archived: Boolean
}

