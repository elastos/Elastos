import { Schema } from 'mongoose'
import { CommentSchema } from './CommentSchema'
import { SubscriberSchema } from './SubscriberSchema'

export const Region = {
  country: String,
  state: String,
  city: String
}

export const Contact = {
  type: Map,
  of: String
}

export const Profile = {
  firstName: String,
  lastName: String,
  avatar: String,
  avatarFilename: String,
  avatarFileType: String,
  banner: String,
  bannerFilename: String,
  bannerFileType: String,
  gender: String,
  birth: Date,
  timezone: String,
  region: Region,
  country: String,
  state: String,
  city: String,
  profession: String,

  telegram: String,
  reddit: String,
  wechat: String,
  twitter: String,
  facebook: String,
  github: String,
  linkedin: String,

  portfolio: String,
  skillset: [String],
  bio: String,
  motto: String,
  beOrganizer: Boolean,
  isDeveloper: Boolean,
  source: String,
  walletAddress: String
}

export const WorkProject = {
  startTime: Date,
  endTime: Date,
  description: String,
  name: String
}

export const WorkAbout = {
  status: String, // employed, student, etc
  employment: String, // company if employed / school if student
  skill: [String],
  project: [WorkProject],
  resume: String,

  notes: String // private internal notes visible only to admin/council
}

// amount is ELA * 1000
export const ELA = {
  address: String,
  amount: Schema.Types.Number
}

export const VotePower = {
  amount: Number,
  expired: Date
}

const did = {
  id: String,
  message: String,
  compressedPublicKey: String,
  avatar: String,
  didName: String,
  _id: false
}

export const User = {
  did,
  username: {
    type: String,
    required: true,
    index: true,
    unique: true
  },
  password: {
    type: String,
    required: true
  },
  salt: {
    type: String,
    required: true
  },

  // let's keep this on the root object
  email: String,
  profile: Profile,
  defaultLanguage: String,
  workAbout: WorkAbout,

  // resetToken, ensure this is never returned
  resetToken: String,

  // constants.USER_ROLE
  role: String,

  // constants.USER_EMPOWER
  empower: String,

  elaOwed: [ELA],

  notes: String, // private internal notes visible only to admin/council

  // admin or council approved max event budget, defaults to 0
  // decreases upon usage
  elaBudget: [ELA],

  votePower: [VotePower],
  votePowerAmount: {
    // TODO auto calculate with votePower
  },
  active: {
    type: Boolean,
    default: false
  },
  logins: [Date],
  circles: [{ type: Schema.Types.ObjectId, ref: 'team' }],
  comments: [[CommentSchema]],
  subscribers: [SubscriberSchema],
  popupUpdate: {
    type: Boolean,
    default: false
  }
}
