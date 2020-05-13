import { Schema } from 'mongoose'

// TODO: 完善数据结构
export const Secretariat = {
    startDate: {
        type: Date,
        required: true,
    },
    endDate: {
        type: Date,
        required: true,
    },
    status: {
        type: String,
        required: true,
    },
}
