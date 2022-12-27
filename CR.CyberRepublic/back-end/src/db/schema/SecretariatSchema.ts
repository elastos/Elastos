import { Schema } from 'mongoose'

export const Secretariat = {
    user: {
        type: Schema.Types.ObjectId,
        ref: 'users'
    },
    code: {
        type: String,
        // required: true,
    },
    cid: {
        type: String,
        // required: true,
    },
    did: {
        type: String,
        required: true,
    },
    didName: {
        type: String,
        // required: true,
    },
    avatar: {
        type: String,
    },
    address: {
        type: String,
        // required: true,
    },
    location: {
        type: Number,
        // required: true,
    },
    birthday: {
        type: Date,
    },
    email: {
        type: String,
    },
    introduction: {
        type: String,
    },
    wechat: {
        type: String,
    },
    weibo: {
        type: String,
    },
    facebook: {
        type: String,
    },
    microsoft: {
        type: String,
    },
    startDate: {
        type: Date,
        required: true,
    },
    endDate: {
        type: Date,
    },
    status: {
        type: String,
        required: true,
    }
}
