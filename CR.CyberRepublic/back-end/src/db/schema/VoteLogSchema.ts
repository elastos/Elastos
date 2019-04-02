/**
 * This collection is for all transactions involving EPV (Elastos Voting Power)
 *
 * The creation must be tied to the task that created it, this is meant as a ledger
 * so depreciation is a separate entry against a user
 *
 * This is for v1.5
 */
import {Schema} from 'mongoose'

/**
 * This collection is for all transactions involving ELA
 */
export const VoteLog = {

    /**
     * The source of the transaction if any
     */
    taskId: Schema.Types.ObjectId,
    communityId: Schema.Types.ObjectId,
    userId: Schema.Types.ObjectId,

    /**
     *
     */
    amount: {

    },

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
    createdAt: {
        type : Date,
        required : true,
        default : Date.now,
        min : Date.now

    },
    createdBy: Schema.Types.ObjectId,

    updatedAt: {
        type : Date,
        required : false,
        min : Date.now
    }
}
