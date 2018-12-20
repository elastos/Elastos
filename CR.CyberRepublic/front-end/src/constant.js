import * as _ from 'lodash'

const create = (list) => {
    return _.zipObject(list, list)
}

export const USER_ROLE = {
    MEMBER : 'MEMBER',
    LEADER : 'LEADER',
    ADMIN : 'ADMIN',
    COUNCIL: 'COUNCIL'
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
    MEMBER : 'MEMBER',
    OWNER : 'OWNER'
};

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

export const SKILLSET_TYPE = create(['CPP', 'JAVASCRIPT', 'GO', 'PYTHON', 'JAVA', 'SWIFT'])
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
export const TEAM_SUBCATEGORY = create(['ESSENTIAL', 'ADVANCED', 'SERVICES', 'DEVELOPER'])
// Images
export const USER_AVATAR_DEFAULT = '/assets/images/user_blurred_white.png'
export const TASK_AVATAR_DEFAULT = '/assets/images/Elastos_Logo.png'
export const TEAM_AVATAR_DEFAULT = '/assets/images/team_blurred.svg'

export const CVOTE_STATUS = create(['DRAFT', 'PROPOSED', 'ACTIVE', 'REJECT', 'FINAL', 'DEFERRED'])

export const USER_SKILLSET = {
    BUSINESS: create(['VIRTUAL_ASSISTANT', 'DATA_ENTRY', 'MARKET_RESEARCH', 'BUSINESS_PLANS',
        'LEGAL_CONSULTING', 'FINANCIAL_CONSULTING', 'PRESENTATION']),
    DESIGN: create(['LOGO_DESIGN', 'FLYERS', 'PACKAGING', 'ILLUSTRATION', 'INFOGRAPHIC', 'PRODUCT_DESIGN',
        'MERCHANDISE', 'PHOTOSHOP']),
    DEVELOPER: {
        ...SKILLSET_TYPE,
        ...create(['SOFTWARE_TESTING'])
    },
    MARKETING: create(['SOCIAL_MEDIA_MARKETING', 'SEO', 'CONTENT_MARKETING', 'VIDEO_MARKETING',
        'EMAIL_MARKETING', 'MARKETING_STRATEGY', 'WEB_ANALYTICS', 'ECOMMERCE', 'MOBILE_ADVERTISING']),
    MUSIC: create(['VOICE_OVER', 'MIXING', 'MUSIC_PRODUCTION']),
    WRITING: create(['TRANSLATION', 'PRODUCT_DESCRIPTIONS', 'WEBSITE_CONTENT', 'TECHNICAL_WRITING',
        'PROOFREADING', 'CREATIVE_WRITING', 'ARTICLES_WRITING', 'SALES_COPY', 'PRESS_RELEASES',
        'LEGAL_WRITING']),
    VIDEO: create(['INTROS', 'LOGO_ANIMATION', 'PROMO_VIDEOS', 'VIDEO_ADS', 'VIDEO_EDITING',
        'VIDEO_MODELING', 'PRODUCT_PHOTO']),
}

export const SORT_ORDER = create(['ASC', 'DESC'])

export const USER_PROFESSION = create(['ENGINEERING', 'COMPUTER_SCIENCE', 'PRODUCT_MANAGEMENT',
    'ART_DESIGN', 'SALES', 'MARKETING', 'BUSINESS_FINANCE', 'ENTREPRENEUR', 'STUDENT',
    'HEALTH_MEDICINE', 'LITERATURE_WRITING', 'TRANSLATION', 'LAW', 'ECONOMICS', 'MANAGEMENT', 'OTHER'])
