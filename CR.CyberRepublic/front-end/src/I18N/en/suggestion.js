import { ABSTRACT_MAX_WORDS } from '@/constant'
import CrProposalPageSvg from '../../module/page/suggestion/list/svg/CrProposalPageSvg';

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
    summary: 'Summary'
  },
  btn: {
    makeIntoProposal: 'Make Into Proposal',
    needBgInvestigation: 'Need Background Investigation',
    needAdvisory: 'Need Advisory'
  },
  status: {
    posted: 'Posted',
    underConsideration: 'Under Council Consideration',
    moreInfoRequired: 'More Info Required',
    referred: 'Referred in Proposal'
  },
  form: {
    button: {
      continue: 'Continue',
      cancel: 'Cancel',
      saveDraft: 'Save as Draft',
      save: 'Save & Post'
    },
    fields: {
      title: 'Title',
    },
    type: {
      newMotion: 'New Motion',
      motionAgainst: 'Motion Against',
      anythingElse: 'Anything Else'
    },
    note: {
      type: 'Select a suggestion type.',
      abstract: 'Provide a brief description of the proposal content (no more than 200 words).',
      goal: 'Describe the desired results achieved by implementing the proposal. Goals should be clear and measurable.',
      motivation: 'Describe why this proposal is proposed. Motivation is critical for proposals that try to improve Elastos or CR. It should be clearly explained here why the existing mechanism is not enough to solve the problems the proposal wants to solve.',
      motivationHighlight: 'Proposals with insufficient motivation are highly likely to be rejected.',
      relevance: 'If the proposal is related to other CRC proposals, the proposal number of each related proposal should be provided here. The relationship between the proposal and each related proposal should be described. If there are conflicts with Elastos technology or other CRC proposals, the proposal must describe these conflicts and explain how to deal with them.',
      budget: 'If the implementation of the proposal requires financial support from the CRC, describe the overall budget and expenditure plan. This financial plan should be aligned with the implementation plan.',
      type: 'Select a proposal type.',
      plan: 'Describe what methods and processes will be used to achieve goals, and a brief introduction of the executing person or team should be listed here as well. If proposal has a long implementation timeline, it should set some checkpoints in the implementation process. The interval between two checkpoints should be no more than 3 months. The checkpoints should be clear and measurable as the proposed goals.',
      tracking: 'This part is updated by the proposer according to the progress of the proposal, including the achievement of goal and budget usage. It is used to present the implementation status of proposal according to the checkpoints in the implementation plan or the goals of the proposal. The CRC Secretariat is responsible to review and verify this part.',
      summary: 'When proposal is completed, its proposer should submit a summary of the proposal implementation, including the achievement of goals and financial report. The CRC Secretariat is responsible for the review of this part.'
    },
    error: {
      required: 'This field is required',
      tooLong: 'This field is too long',
      [`limit${ABSTRACT_MAX_WORDS}`]: `You can only type ${ABSTRACT_MAX_WORDS} words max.`
    }
  },
  modal: {
    addTagComment: 'Add Comment',
    confirm: 'Confirm',
    cancel: 'Cancel',
    pleaseUpdate:
      'Please update the Suggestion with the requested info and notify the council/community through a comment.',
    commentsFromCouncil: 'Comments from Council/Admin:',
    consideration: 'Are you sure to mark this suggestion as under council consideration?'
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
    notify: 'Email was sent to secretary.'
  },
  popover: {
    email: 'Email',
    name: 'Name',
    copy: 'Copy',
    viewProfile: 'View Profile'
  },
  header: {
    suggestion: 'suggestion',
    crCouncil: 'cr council',
    crProposalPage: 'cr proposal page',
    approvedProposal: 'approved proposal'
  }
}
