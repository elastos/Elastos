import { Schema } from 'mongoose'

export const CouncilMembers = {
    user: {
        type: Schema.Types.ObjectId,
        ref: 'users'
    },
    code: {
        type: String,
        required: true,
    },
    cid: {
        type: String,
    },
    did: {
        type: String,
        required: true,
    },
    didName: {
        type: String,
    },
    email: {
        type: String
    },
    avatar: {
        type: String,
    },
    address: {
        type: String,
    },
    introduction: {
        type: String,
    },
    votes: {
        type: Number,
    },
    impeachmentVotes: {
        type: Number,
    },
    location: {
        type: Number,
        required: true,
    },
    depositAmount: {
        type: String,
    },
    depositAddress: {
        type: String,
    },
    penalty: {
        type: String,
    },
    status: {
        type: String,
        required: true,
    },
    index: {
        type: Number,
        required: true,
    }
}

export const Council = {
    index: {
        type: Number,
        required: true,
    },
    startDate: {
        type: Date,
    },
    endDate: {
        type: Date,
    },
    // startHeight: {
    //     type: Number,
    //     required: true,
    // },
    // endHeight: {
    //     type: Number,
    // },
    status: {
        type: String,
        required: true,
    },
    circulatingSupply: {
        type: Number,
        required: true
    },
    // current height
    height: {
        type: Number,
        required: true,
    },
    councilMembers: [CouncilMembers],
}
