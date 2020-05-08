import * as _ from 'lodash'

const create = (list) => {
  return _.zipObject(list, list)
}

export const DATE_FORMAT = 'MMM D, YYYY'
export const ABSTRACT_MAX_WORDS = 200
export const ELIP_DESC_MAX_WORDS = 3000

export const CR_LINKS = {
  FACEBOOK: 'https://www.facebook.com/ElastosCyberRepublic',
  GITHUB: 'https://github.com/cyber-republic',
  INSTAGRAM: 'https://www.instagram.com/cyberrepublic',
  LINKEDIN: 'https://www.linkedin.com/company/cyber-republic',
  REDDIT: 'https://www.reddit.com/r/CyberRepublic',
  TELEGRAM: 'https://t.me/elastosgroup',
  TWITTER: 'https://twitter.com/cyber__republic',
  YOUTUBE: 'https://www.youtube.com/channel/UCjHthS-zJr0axZF5Iw8En-w'
}

export const ELASTOS_LINKS = {
  EXPLORER: 'https://blockchain.elastos.org/status',
  GITHUB: 'https://github.com/elastos',
  NEWS: 'https://news.elastos.org',
  WALLET: 'https://wallet.elastos.org',
  WWW: 'https://www.elastos.org'
}

export const USER_ROLE = {
  ADMIN: 'ADMIN',
  COUNCIL: 'COUNCIL',
  SECRETARY: 'SECRETARY',
  CUSTOM: 'CUSTOM',
  MEMBER: 'MEMBER',
  LEADER: 'LEADER'
}

export const USER_ROLE_TO_TEXT = {
  ADMIN: 'Admin',
  COUNCIL: 'Council',
  SECRETARY: 'Secretary General',
  CUSTOM: 'Custom',
  MEMBER: 'User',
  LEADER: 'Leader'
}

export const USER_EMPOWER_TYPE = {
  MARKETING: 'MARKETING',
  PRODUCT_MANAGER: 'PRODUCT_MANAGER',
  LEGAL: 'LEGAL',
  DESIGNER: 'DESIGNER',
  MEDIA_PRODUCER: 'MEDIA_PRODUCER',
  WRITER: 'WRITER',
  PARTNERSHIP: 'PARTNERSHIP',
  INVESTMENTS: 'INVESTMENTS',
  BUSINESS_DEVELOPMENT: 'BUSINESS_DEVELOPMENT',
  MEDIA: 'MEDIA',
  WRITER_CONTENT: 'WRITER_CONTENT',
  WRITER_TECHNICAL: 'WRITER_TECHNICAL',
  LEAD_DEVELOPER_SUPPORT: 'LEAD_DEVELOPER_SUPPORT',
  DAPP_ANALYST: 'DAPP_ANALYST',
  ADMINISTRATOR: 'ADMINISTRATOR',
  HR_DIRECTOR: 'HR_DIRECTOR',
  SECURITY: 'SECURITY',
  OPEN_TITLE: 'OPEN_TITLE',
  LEAD_TRANSLATOR: 'LEAD_TRANSLATOR',
  DAPP_CONSULTANT: 'DAPP_CONSULTANT',
  REGIONAL_EVANGELIST: 'REGIONAL_EVANGELIST'
}

export const USER_LANGUAGE = {
  en: 'en',
  zh: 'zh'
}

export const USER_GENDER = {
  MALE: 'male',
  FEMALE: 'female',
  OTHER: 'other'
}

export const TEAM_ROLE = {
  MEMBER: 'MEMBER',
  OWNER: 'OWNER'
}

export const TASK_CATEGORY = {
  GENERAL: 'GENERAL',
  SOCIAL: 'SOCIAL',
  DEVELOPER: 'DEVELOPER',
  LEADER: 'LEADER',
  CR100: 'CR100'
}

export const TASK_TYPE = {
  TASK: 'TASK',
  SUB_TASK: 'SUB_TASK',
  PROJECT: 'PROJECT',
  EVENT: 'EVENT'
}

export const TASK_EVENT_DATE_TYPE = {
  NOT_APPLICABLE: 'NOT_APPLICABLE',
  TENTATIVE: 'TENTATIVE',
  CONFIRMED: 'CONFIRMED'
}

export const TASK_STATUS = {
  // PROPOSAL: 'PROPOSAL',

  CREATED: 'CREATED', // if no ELA
  PENDING: 'PENDING', // if ELA > 0

  APPROVED: 'APPROVED', // Approved by admin

  ASSIGNED: 'ASSIGNED', // when max candidates are accepted or auto assigned

  // in between ASSIGNED and SUBMITTED, individual task candidates
  // can mark their completion which is recorded in the array candidateCompleted
  // this is only for reference, the task is not fully completed until the owner

  // owner acknowledges task is done - by enough parties (note it does not have to be all)
  SUBMITTED: 'SUBMITTED',

  SUCCESS: 'SUCCESS', // when admin accepts it as complete
  DISTRIBUTED: 'DISTRIBUTED', // when admin distributes ELA rewards
  CANCELED: 'CANCELED',

  // TODO: application deadline passed without any applicants
  EXPIRED: 'EXPIRED'
}

export const TASK_CANDIDATE_TYPE = {
  USER: 'USER',
  TEAM: 'TEAM'
}

export const TASK_CANDIDATE_CATEGORY = {
  RSVP: 'RSVP'
}

export const TASK_CANDIDATE_STATUS = {
  // NOT_REQUIRED: 'NOT_REQUIRED',
  PENDING: 'PENDING',
  APPROVED: 'APPROVED',
  REJECTED: 'REJECTED'
}

export const COMMUNITY_TYPE = {
  COUNTRY: 'COUNTRY',
  STATE: 'STATE',
  CITY: 'CITY',
  REGION: 'REGION',
  SCHOOL: 'SCHOOL'
}

export const TRANS_STATUS = {
  PENDING: 'PENDING',
  CANCELED: 'CANCELED',
  FAILED: 'FAILED',
  SUCCESSFUL: 'SUCCESSFUL'
}

export const CONTRIB_CATEGORY = {
  BLOG: 'BLOG',
  VIDEO: 'VIDEO',
  PODCAST: 'PODCAST',
  OTHER: 'OTHER'
}

export const DEFAULT_IMAGE = {
  TASK: '/assets/images/task_thumbs/12.jpg',
  UNSET_LEADER: '/assets/images/User_Avatar_Other.png'
}

export const SUBMISSION_TYPE = {
  BUG: 'BUG',
  SECURITY_ISSUE: 'SECURITY_ISSUE',
  SUGGESTION: 'SUGGESTION',
  ADD_COMMUNITY: 'ADD_COMMUNITY',
  OTHER: 'OTHER',
  FORM_EXT: 'FORM_EXT',
  EMPOWER_35: 'EMPOWER_35'
}

export const SUBMISSION_CAMPAIGN = {
  COMMUNITY_ORGANIZER: 'COMMUNITY_ORGANIZER',
  ANNI_2008: 'ANNI_2008',
  ANNI_VIDEO_2008: 'ANNI_VIDEO_2008',
  EMPOWER_35: 'EMPOWER_35'
}

export const SKILLSET_TYPE = create([
  'CPP',
  'JAVASCRIPT',
  'GO',
  'PYTHON',
  'JAVA',
  'SWIFT'
])
export const TEAM_TASK_DOMAIN = create([
  'AUTHENTICITY',
  'CURRENCY',
  'EXCHANGE',
  'FINANCE',
  'GAMING',
  'IOT',
  'MEDIA',
  'SOCIAL',
  'SOVEREIGNTY'
])
export const TEAM_USER_STATUS = create(['NORMAL', 'PENDING', 'REJECT'])
export const TEAM_TYPE = create(['TEAM', 'CRCLE'])
export const TEAM_SUBCATEGORY = create([
  'ESSENTIAL',
  'ADVANCED',
  'SERVICES',
  'DEVELOPER'
])
// Images
export const USER_AVATAR_DEFAULT = '/assets/images/default_avatar.png'
export const TASK_AVATAR_DEFAULT = '/assets/images/Elastos_Logo.png'
export const TEAM_AVATAR_DEFAULT = '/assets/images/team_blurred.svg'

// council vote
export const CVOTE_STATUS = create([
  'DRAFT',
  'PROPOSED',
  'ACTIVE',
  'REJECT',
  'FINAL',
  'DEFERRED',
  'INCOMPLETED',
  'NOTIFICATION',
  'VETOED'
])

export const CVOTE_STATUS_TEXT = {
  DRAFT: 'DRAFT',
  PROPOSED: 'PROPOSED',
  ACTIVE: 'PASSED',
  REJECT: 'REJECTED',
  DEFERRED: 'DEFERRED',
  FINAL: 'FINAL',
  INCOMPLETED: 'INCOMPLETED'
}

export const CVOTE_CHAIN_STATUS = {
  CHAINED: 'chained',
  UNCHAIN: 'unchain',
  CHAINING: 'chaining',
  FAILED: 'failed'
}

export const CONTENT_TYPE = create(['MARKDOWN', 'HTML'])

export const CVOTE_RESULT = {
  SUPPORT: 'support',
  REJECT: 'reject',
  ABSTENTION: 'abstention',
  UNDECIDED: 'undecided'
}

export const CVOTE_RESULT_COLOR = {
  [CVOTE_RESULT.UNDECIDED]: '#CED6E3',
  [CVOTE_RESULT.ABSTENTION]: '#0F2631',
  [CVOTE_RESULT.SUPPORT]: '#1DE9B6',
  [CVOTE_RESULT.REJECT]: '#BE1313'
}

export const CVOTE_TRACKING_STATUS = create([
  'DRAFT',
  'REVIEWING',
  'PUBLISHED',
  'REJECT'
])
export const CVOTE_SUMMARY_STATUS = create([
  'DRAFT',
  'REVIEWING',
  'PUBLISHED',
  'REJECT'
])

export const CVOTE_TRACKING_STATUS_TEXT = {
  [CVOTE_TRACKING_STATUS.DRAFT]: 'DRAFT',
  [CVOTE_TRACKING_STATUS.REVIEWING]: 'REVIEWING',
  [CVOTE_TRACKING_STATUS.PUBLISHED]: 'APPROVED',
  [CVOTE_TRACKING_STATUS.REJECT]: 'REJECTED'
}

export const RESOURCE_TYPE_TO_TEXT = {
  CVote: 'Proposal'
}

export const avatar_map = {
  'Kevin Zhang':
    'https://s3-ap-southeast-1.amazonaws.com/s3-cr-asia-prod/44f5fe8e-f062-41e4-ba5b-1dbbea4e63b0_kevin.jpeg',
  'Fay Li':
    'https://s3-ap-southeast-1.amazonaws.com/s3-cr-asia-prod/37706672-efa7-4a7a-8453-200f3ab615d9_IMG_4320.JPG',
  'Yipeng Su':
    'https://s3-ap-southeast-1.amazonaws.com/s3-cr-asia-prod/19691cdc-913f-40d1-9d1d-f88b709fcecf_yipeng.jpeg'
}

export const CVOTE_TYPE = {
  1: 'New Motion',
  2: 'Motion Against',
  3: 'Anything Else'
}

export const USER_SKILLSET = {
  BUSINESS: create([
    'VIRTUAL_ASSISTANT',
    'DATA_ENTRY',
    'MARKET_RESEARCH',
    'BUSINESS_PLANS',
    'LEGAL_CONSULTING',
    'FINANCIAL_CONSULTING',
    'PRESENTATION'
  ]),
  DESIGN: create([
    'LOGO_DESIGN',
    'FLYERS',
    'PACKAGING',
    'ILLUSTRATION',
    'INFOGRAPHIC',
    'PRODUCT_DESIGN',
    'MERCHANDISE',
    'PHOTOSHOP'
  ]),
  DEVELOPER: {
    ...SKILLSET_TYPE,
    ...create(['SOFTWARE_TESTING'])
  },
  MARKETING: create([
    'SOCIAL_MEDIA_MARKETING',
    'SEO',
    'CONTENT_MARKETING',
    'VIDEO_MARKETING',
    'EMAIL_MARKETING',
    'MARKETING_STRATEGY',
    'WEB_ANALYTICS',
    'ECOMMERCE',
    'MOBILE_ADVERTISING'
  ]),
  MUSIC: create(['VOICE_OVER', 'MIXING', 'MUSIC_PRODUCTION']),
  WRITING: create([
    'TRANSLATION',
    'PRODUCT_DESCRIPTIONS',
    'WEBSITE_CONTENT',
    'TECHNICAL_WRITING',
    'PROOFREADING',
    'CREATIVE_WRITING',
    'ARTICLES_WRITING',
    'SALES_COPY',
    'PRESS_RELEASES',
    'LEGAL_WRITING'
  ]),
  VIDEO: create([
    'INTROS',
    'LOGO_ANIMATION',
    'PROMO_VIDEOS',
    'VIDEO_ADS',
    'VIDEO_EDITING',
    'VIDEO_MODELING',
    'PRODUCT_PHOTO'
  ])
}

export const SORT_ORDER = create(['ASC', 'DESC'])

export const USER_PROFESSION = create([
  'ENGINEERING',
  'COMPUTER_SCIENCE',
  'PRODUCT_MANAGEMENT',
  'ART_DESIGN',
  'SALES',
  'MARKETING',
  'BUSINESS_FINANCE',
  'ENTREPRENEUR',
  'STUDENT',
  'HEALTH_MEDICINE',
  'LITERATURE_WRITING',
  'TRANSLATION',
  'LAW',
  'ECONOMICS',
  'MANAGEMENT',
  'OTHER'
])

// suggestion
export const SUGGESTION_STATUS = create(['ACTIVE', 'ABUSED', 'ARCHIVED'])

export const SUGGESTION_ABUSED_STATUS = create(['REPORTED', 'HANDLED'])

export const SUGGESTION_TAG_TYPE = create([
  'UNDER_CONSIDERATION',
  'INFO_NEEDED'
])

export const SUGGESTION_SEARCH_FILTERS = create([
  'TITLE',
  'NUMBER',
  'ABSTRACT',
  'EMAIL',
  'NAME'
])

export const SUGGESTION_BUDGET_TYPE = create([
  'ADVANCE',
  'COMPLETION',
  'CONDITIONED'
])

// elip
export const ELIP_STATUS = create([
  'PERSONAL_DRAFT',
  'WAIT_FOR_REVIEW',
  'REJECTED',
  'DRAFT',
  'CANCELLED',
  'FINAL_REVIEW',
  'SUBMITTED_AS_PROPOSAL'
])

export const ELIP_REVIEW_STATUS = create(['APPROVED', 'REJECTED'])
export const ELIP_VOTE_STATUS = create(['YES', 'OPPOSE', 'ABSTAIN'])
export const ELIP_VOTE_STATUS_COLOR = {
  [ELIP_VOTE_STATUS.YES]: '#008D85',
  [ELIP_VOTE_STATUS.OPPOSE]: '#BE1313',
  [ELIP_VOTE_STATUS.ABSTAIN]: '#C4C4C4'
}
export const ELIP_TYPE = create(['STANDARD_TRACK', 'PROCESS', 'INFORMATIONAL'])
export const ELIP_NUMBER_TYPE = ['4', '5', '6']

export const MILESTONE_STATUS = create([
  'WAITING_FOR_REQUEST',
  'REJECTED',
  'WAITING_FOR_APPROVAL',
  'WAITING_FOR_WITHDRAWAL',
  'WITHDRAWN'
])
export const REVIEW_OPINION = create(['REJECTED', 'APPROVED'])
