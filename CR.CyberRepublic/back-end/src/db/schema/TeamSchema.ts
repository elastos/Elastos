import {Schema} from 'mongoose'
import {PictureSchema} from './PictureSchema'
import {CommentSchema} from './CommentSchema'

export const TeamProfile = {
    description: String,
    logo : String
}

export const Team = {
    name: {
        type : String,
        required : true
    },
    name_zh: String,
    metadata: {
        type: Map,
        of: String
    },
    type : {
        type : String,
        required : true
    },
    tags: [String],
    profile: TeamProfile,
    domain: [String],
    recruitedSkillsets: [String],
    subcategory: String,
    owner: {type: Schema.Types.ObjectId, ref: 'users'},
    members: [{type: Schema.Types.ObjectId, ref: 'user_team'}],
    pictures: [PictureSchema],
    comments: [[CommentSchema]],
    tasks: {
        count: Number,
        budget: {
            usd: Number,
            ela: Number
        }
    }
}

export const User_Team = {
    status: {
        type: String
    },
    level: String,
    role: String,
    title: String,
    apply_reason : String,
    team: {type: Schema.Types.ObjectId, ref: 'team'},
    user: {type: Schema.Types.ObjectId, ref: 'users'},
    comments: [[CommentSchema]]
}
