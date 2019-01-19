import * as _ from 'lodash';

const create = (constant_list: string[]): any => {
    const map = {};
    _.each(constant_list, (key)=>{
        map[key] = key;
    });

    return map;
}

export const USER_ROLE = {
    MEMBER : 'MEMBER',
    LEADER : 'LEADER',
    ADMIN : 'ADMIN',
    COUNCIL: 'COUNCIL',
    SECRETARY: 'SECRETARY'
}

export const USER_LANGUAGE = {
    en: 'en',
    zh: 'zh'
}

export const TASK_TYPE = {
    TASK: 'TASK',
    SUB_TASK: 'SUB_TASK',
    PROJECT: 'PROJECT',
    EVENT: 'EVENT'
}

export const TASK_CATEGORY = {
    GENERAL: 'GENERAL',
    SOCIAL: 'SOCIAL',
    DEVELOPER: 'DEVELOPER',
    LEADER: 'LEADER',
    CR100: 'CR100'
}

export const TASK_STATUS = {
    PROPOSAL: 'PROPOSAL',
    CREATED: 'CREATED',
    PENDING: 'PENDING',
    APPROVED: 'APPROVED',
    ASSIGNED: 'ASSIGNED',

    // in between ASSIGNED and SUBMITTED, individual task candidates
    // can mark their completion which is recorded in the array candidateCompleted
    // this is only for reference, the task is not fully completed until the owner

    // owner acknowledges task is done - by enough parties (note it does not have to be all)
    SUBMITTED: 'SUBMITTED',

    SUCCESS: 'SUCCESS', // when admin accepts it as complete
    DISTRIBUTED: 'DISTRIBUTED', // when admin distributes ELA rewards
    CANCELED: 'CANCELED',
    EXPIRED: 'EXPIRED'
}

export const TASK_CANDIDATE_TYPE = {
    USER: 'USER',
    TEAM: 'TEAM'
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

// log
export const LOG_TYPE = {
    'APPLY_TEAM' : 'apply_team'
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
    'MEDIA',
    'IOT',
    'AUTHENTICITY',
    'CURRENCY',
    'GAMING',
    'FINANCE',
    'SOVEREIGNTY',
    'SOCIAL',
    'EXCHANGE'
])

export const TEAM_SUBCATEGORY = create(['ESSENTIAL', 'ADVANCED', 'SERVICES'])
export const TEAM_ROLE = create(['MEMBER', 'LEADER'])
export const TEAM_USER_STATUS = create(['NORMAL', 'PENDING', 'REJECT'])
export const TEAM_TYPE = create(['TEAM', 'CRCLE'])
export const TASK_CANDIDATE_CATEGORY = {
    RSVP: 'RSVP'
}

export const CVOTE_STATUS = create(['DRAFT', 'PROPOSED', 'ACTIVE', 'REJECT', 'FINAL', 'DEFERRED']);
export const CVOTE_RESULT = {
    SUPPORT: 'support',
    REJECT: 'reject',
    ABSTENTION: 'abstention',
    UNDECIDED: 'undecided',
};
export const CVOTE_EXPIRATION = 1000*60*60*24*7;

export const USER_SKILLSET = {
    DESIGN: create(['LOGO_DESIGN', 'FLYERS', 'PACKAGING', 'ILLUSTRATION', 'INFOGRAPHIC', 'PRODUCT_DESIGN',
        'MERCHANDISE', 'PHOTOSHOP']),
    MARKETING: create(['SOCIAL_MEDIA_MARKETING', 'SEO', 'CONTENT_MARKETING', 'VIDEO_MARKETING',
        'EMAIL_MARKETING', 'MARKETING_STRATEGY', 'WEB_ANALYTICS', 'ECOMMERCE', 'MOBILE_ADVERTISING']),
    WRITING: create(['TRANSLATION', 'PRODUCT_DESCRIPTIONS', 'WEBSITE_CONTENT', 'TECHNICAL_WRITING',
        'PROOFREADING', 'CREATIVE_WRITING', 'ARTICLES_WRITING', 'SALES_COPY', 'PRESS_RELEASES',
        'LEGAL_WRITING']),
    VIDEO: create(['INTROS', 'LOGO_ANIMATION', 'PROMO_VIDEOS', 'VIDEO_ADS', 'VIDEO_EDITING',
        'VIDEO_MODELING', 'PRODUCT_PHOTO']),
    MUSIC: create(['VOICE_OVER', 'MIXING', 'MUSIC_PRODUCTION']),
    DEVELOPER: {
        ...SKILLSET_TYPE,
        ...create(['SOFTWARE_TESTING'])
    },
    BUSINESS: create(['VIRTUAL_ASSISTANT', 'DATA_ENTRY', 'MARKET_RESEARCH', 'BUSINESS_PLANS',
        'LEGAL_CONSULTING', 'FINANCIAL_CONSULTING', 'PRESENTATION'])
}

// mongo do not support ASC and DESC options
export const SORT_ORDER = {
    ASC: 1,
    DESC: -1
}

export const USER_PROFESSION = create(['ENGINEERING', 'COMPUTER_SCIENCE', 'PRODUCT_MANAGEMENT',
    'ART_DESIGN', 'SALES', 'MARKETING', 'BUSINESS_FINANCE', 'ENTREPRENEUR', 'STUDENT',
    'HEALTH_MEDICINE', 'LITERATURE_WRITING', 'TRANSLATION', 'LAW', 'ECONOMICS', 'MANAGEMENT'])

export const SUGGESTION_STATUS = create(['ACTIVE', 'ABUSED', 'ARCHIVED'])

export const SUGGESTION_ABUSED_STATUS = create(['REPORTED', 'HANDLED'])

// DB sensitive data we do not want to explosure
export const DB_EXCLUDED_FIELDS = {
    USER: {
        SENSITIVE: '-password -salt -email -resetToken',
    },
}

export const DB_SELECTED_FIELDS = {
    USER: {
        NAME: 'profile.firstName profile.lastName username'
    },
}

export const COUNCIL_MEMBERS = {
    '5bcf21f030826d68a940b017': 'Yipeng Su',
    // '5c2f5a15f13d65008969be61': 'Feng Zhang',
    '5b367c128f23a70035d35425': 'Fay Li',
    '5b28be2784f6f900350d30b9': 'Kevin Zhang',
}
export const COUNCIL_MEMBER_IDS = _.keys(COUNCIL_MEMBERS)
