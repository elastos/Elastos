import {
  ABSTRACT_MAX_WORDS,
  SUGGESTION_STATUS,
  SUGGESTION_BUDGET_TYPE
} from '@/constant'

export default {
  title: {
    add: 'ADD A SUGGESTION',
    edit: 'EDIT SUGGESTION'
  },
  fields: {
    title: 'Title',
    preamble: 'Preamble',
    preambleSub: {
      suggestion: 'Suggestion',
      title: 'Title',
      creator: 'Creator',
      status: 'Status',
      created: 'Created',
      updated: 'Updated',
      signature: 'Signature'
    },
    abstract: 'Abstract',
    goal: 'Goal',
    motivation: 'Motivation',
    relevance: 'Relevance',
    budget: 'Budget',
    type: 'Type',
    plan: 'Implementation Plan',
    vote: 'Vote',
    tracking: 'Tracking',
    summary: 'Summary',
    status: 'Suggestion Status',
    budgetRequested: 'Budget Requested',
    creationDate: 'Creation Date',
    author: 'Author'
  },
  btn: {
    makeIntoProposal: 'Make Into Proposal',
    needDueDiligence: 'Need Due Diligence',
    needAdvisory: 'Need Advisory',
    signSuggetion: 'Sign Suggestion',
    associateDid: 'Associate DID',
    viewOldData: 'View old data',
    viewNewData: 'View new data'
  },
  status: {
    posted: 'Posted',
    underConsideration: 'Under Council Consideration',
    moreInfoRequired: 'More Info Required',
    referred: 'Referred in Proposal',
    [SUGGESTION_STATUS.ACTIVE]: 'Active',
    [SUGGESTION_STATUS.ABUSED]: 'Abused',
    [SUGGESTION_STATUS.ARCHIVED]: 'Archived'
  },
  form: {
    search: 'Search Suggestions',
    button: {
      continue: 'Continue',
      cancel: 'Cancel',
      saveDraft: 'Save as Draft',
      save: 'Save & Post',
      update: 'Update',
      create: 'Create',
      discardChanges: 'Discard changes',
      revertVersion: 'Revert to this version',
      showVersion: 'Show version history'
    },
    fields: {
      title: 'Title'
    },
    type: {
      newMotion: 'New Motion',
      motionAgainst: 'Motion Against',
      anythingElse: 'Anything Else'
    },
    note: {
      type: 'Select a suggestion type.',
      abstract: `Provide a brief description of the proposal content (no more than 200 words).`,
      goal: `Describe the desired results achieved by implementing the proposal. Goals should be clear and measurable.`,
      motivation: `Describe why this proposal is proposed. Motivation is critical for proposals that try to improve Elastos or CR. It should be clearly explained here why the existing mechanism is not enough to solve the problems the proposal wants to solve.`,
      motivationHighlight: `Proposals with insufficient motivation are highly likely to be rejected.`,
      relevance: `If the proposal is related to other CRC proposals, the proposal number of each related proposal should be provided here. The relationship between the proposal and each related proposal should be described. If there are conflicts with Elastos technology or other CRC proposals, the proposal must describe these conflicts and explain how to deal with them.`,
      budget: `If the implementation of the proposal requires financial support from the CRC, describe the overall budget and expenditure plan. This financial plan should be aligned with the implementation plan.`,
      type: 'Select a proposal type.',
      plan: `Describe what methods and processes will be used to achieve goals, and a brief introduction of the executing person or team should be listed here as well. If proposal has a long implementation timeline, it should set some checkpoints in the implementation process. The interval between two checkpoints should be no more than 3 months. The checkpoints should be clear and measurable as the proposed goals.`,
      tracking: `This part is updated by the proposer according to the progress of the proposal, including the achievement of goal and budget usage. It is used to present the implementation status of proposal according to the checkpoints in the implementation plan or the goals of the proposal. The CRC Secretariat is responsible to review and verify this part.`,
      summary: `When proposal is completed, its proposer should submit a summary of the proposal implementation, including the achievement of goals and financial report. The CRC Secretariat is responsible for the review of this part.`
    },
    error: {
      required: 'This field is required',
      tooLong: 'This field is too long',
      [`limit${ABSTRACT_MAX_WORDS}`]: `You can only type ${ABSTRACT_MAX_WORDS} words max.`,
      isNaN: 'Please input valid amount',
      milestones: 'Milestones is empty',
      team: 'Implementation team is empty',
      amount: 'The amount is more than 30% of the total budget',
      previousMilestoneDate: `The date must be later than the previous milestone.`,
      requirePayment: `Project Completion Payment is required before completing a proposal.`,
      elaAddress: 'Invalid ELA address',
      schedule: 'Payment schedule is empty',
      payment: `Project Completion Payment is required, at most one Project Initiation Payment, and each payment must match one milestone.`,
      advance: 'Project initiation payment only apply to the first milestone.',
      completion: `Project completion payment only apply to the last milestone.`,
      conditioned: 'Project milestone payment can not apply to this milestone.',
      isUsed: 'This milestone has been used.',
      exception: 'Something went wrong',
      notEqual: `The toal budget is not equal to the sum of each payment amount.`
    }
  },
  modal: {
    addTagComment: 'Add Comment',
    confirm: 'Confirm',
    cancel: 'Cancel',
    pleaseUpdate: `Please update the Suggestion with the requested info and notify the council/community through a comment.`,
    commentsFromCouncil: 'Comments from Council/Admin:',
    consideration: `Are you sure to mark this suggestion as under council consideration?`,
    signNotice: `Ready for council members to review? Please sign the suggestion now. A suggestion can not be edited if it was signed.`,
    signNow: 'Sign Now',
    signLater: 'Later'
  },
  tag: {
    show: 'Show',
    type: {
      UNDER_CONSIDERATION: 'Under Council Consideration',
      INFO_NEEDED: 'More Info Required',
      ADDED_TO_PROPOSAL: 'Added to Proposal'
    }
  },
  msg: {
    consideration: 'Marked as under council consideration',
    notify: 'Email was sent to secretary.',
    archived: 'Suggestion archived successfully.',
    unarchived: 'Suggestion unarchived successfully.',
    revertVersion: 'Suggestion reverted successfully.',
    madeByOtherCM: `This suggestion had been made into proposal by other council member.`,
    councilQRCode: `Scan the QR code above to make this suggestion into proposal. Then go to the proposal list page to view the proposal when it is saved on the chain.`,
    toChain: 'Saving onto blockchain',
    signQRCode: 'Scan the QR code above to sign your suggestion.',
    associateDidFirst: 'Please associate your DID first.'
  },
  header: {
    suggestion: 'suggestion',
    crCouncil: 'cr council',
    crProposalPage: 'cr proposal page',
    approvedProposal: 'approved proposal'
  },
  search: {
    number: 'Number',
    title: 'Title',
    abstract: 'Abstract',
    email: 'Author email',
    name: 'Author name'
  },
  budget: {
    create: 'Add Payment Line',
    payment: 'Payment',
    type: 'Type',
    amount: 'Amount',
    reasons: 'Reasons',
    criteria: 'Criteria of Payment',
    action: 'Action',
    desc: 'Based on milestone selected below:',
    schedule: 'Payment Schedule',
    total: 'Total Budget',
    address: 'ELA Receive Address',
    tip: 'Please add milestone first',
    [SUGGESTION_BUDGET_TYPE.ADVANCE]: 'Project Initiation Payment',
    [SUGGESTION_BUDGET_TYPE.COMPLETION]: 'Project Completion Payment',
    [SUGGESTION_BUDGET_TYPE.CONDITIONED]: 'Project Milestone Payment',
    goal: 'Goal',
    milestone: 'milestone'
  },
  plan: {
    teamMember: 'Team Member',
    role: 'Role',
    responsibility: 'Responsibility',
    moreInfo: 'More Info',
    createTeamInfo: 'Add Team Member',
    action: 'Action',
    teamInfo: 'Implementation Team',
    milestones: 'Milestones',
    publishDate: 'Publish Date',
    version: 'Version',
    goal: 'Goal',
    showDetail: 'Show Detail',
    hideDetail: 'Hide Detail',
    selectDate: 'Select date',
    milestone: 'Milestone'
  },
  label: {
    hasMadeIntoProposal: 'has made it into'
  }
}
