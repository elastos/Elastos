import {Schema} from 'mongoose'

/**
 * This collection is for all transactions involving ELA
 */
export const Transaction = {

    /**
     * The source of the transaction if any
     */
    taskId: Schema.Types.ObjectId,
    communityId: Schema.Types.ObjectId,
    userId: Schema.Types.ObjectId,

    /**
     * Currently this is only MANUAL
     */
    method : {
        type : String,
        default : 'MANUAL'
    },

    type: {
        type : String
    },

    /**
     * ELA * 1000 - e.g. 0.01 ELA is written as 100
     */
    amount: Schema.Types.Number,

    elaFromAddress: {
        type : String
    },

    elaToAddress: {
        type : String
    },

    elaTransaction: Schema.Types.String,

    // constants.TRANS_STATUS
    status : {
        type : String
    },

    paidBy: Schema.Types.ObjectId,
    paidDate : {
        type : Date,
        required : false,
        min : Date.now
    },


    createdBy: Schema.Types.ObjectId
}
