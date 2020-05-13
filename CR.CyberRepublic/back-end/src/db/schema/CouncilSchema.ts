import { Schema } from 'mongoose'

// TODO: 完善数据结构
export const Council = {
    index: {
        type: Number,
        required: true,
    },
    status: {
        type: String,
        required: true,
    },
    createdAt: {
        type: Date,
        required: true,
    },
}
