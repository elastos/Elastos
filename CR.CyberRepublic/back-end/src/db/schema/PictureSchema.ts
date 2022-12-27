import {Schema} from 'mongoose'
import {ELA, VotePower} from './UserSchema'
import {constant} from '../../constant'

export const PictureSchema = {
    thumbUrl: String,
    name: String,
    url: String,
    uid: String
}
