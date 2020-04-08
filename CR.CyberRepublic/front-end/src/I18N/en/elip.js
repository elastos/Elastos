import {
  ELIP_STATUS,
  ELIP_TYPE,
  ELIP_DESC_MAX_WORDS
} from '@/constant'

export default {
  header: 'ELIPS',
  fields: {
    number: 'Number',
    title: 'Title',
    author: 'Author',
    status: 'Status',
    createdAt: 'Created',
    updatedAt: 'Updated',
    description: 'Description',
    type: 'Type',
    preamble: 'Preamble',
    abstract: 'Abstract',
    specification: 'Specifications',
    motivation: 'Motivation',
    rationale: 'Rationale',
    backwardCompatibility: 'Backward Compatibility',
    referenceImplementation: 'Reference Implementation',
    copyright: 'Copyright/Public Domain',
    review: 'Review',
    vote: 'Council Member Votes',
    preambleItems: {
      elip: 'ELIP',
      title: 'Title',
      author: 'Author',
      discussions: 'Discussions-To',
      status: 'Status',
      type: 'Type',
      created: 'Created',
      requires: 'Requires',
      replaces: 'Replaces',
      superseded: 'Superseded-By'
    },
    filter: 'Filters',
    elipStatus: 'Elip Status',
    creationDate: 'Creation Date'
  },
  status: {
    [ELIP_STATUS.WAIT_FOR_REVIEW]: 'WAIT FOR REVIEW',
    [ELIP_STATUS.REJECTED]: 'REJECTED',
    [ELIP_STATUS.DRAFT]: 'DRAFT',
    [ELIP_STATUS.CANCELLED]: 'CANCELLED',
    [ELIP_STATUS.SUBMITTED_AS_PROPOSAL]: 'SUBMITTED AS PROPOSAL',
    [ELIP_STATUS.PERSONAL_DRAFT]: 'PERSONAL DRAFT',
    [ELIP_STATUS.FINAL_REVIEW]: 'FINAL REVIEW'
  },
  show: 'Show',
  button: {
    add: 'Add ELIP',
    cancel: 'Cancel',
    back: 'Back',
    submit: 'Submit for Review',
    reject: 'Reject',
    approve: 'Approve',
    edit: 'Edit',
    submittedAsProposal: 'Submit as Proposal',
    continue: 'Continue',
    yes: 'Vote Yes',
    oppose: 'Oppose with a Reason',
    abstain: 'Abstain',
    delete: 'Delete',
    exportAsCSV: 'Export as CSV',
    clearFilter: 'Clear Filter',
    applyFilter: 'Apply Filter',
    preview: 'Preview',
    personalDraft: 'Save as Personal Draft',
    saveChanges: 'Save Changes',
    cancelled: 'Mark as Cancelled'
  },
  msg: {
    updated: 'Updated successfully',
    submitted: 'Submitted successfully',
    rejected: 'Rejected successfully',
    approved: 'Approved successfully',
    marked: 'Marked successfully',
    deleted: 'Deleted successfully',
    continue: 'Continue'
  },
  form: {
    note: {
      type: 'Select ELIP Type.',
      abstract: 'Abstract',
      specification: 'Specification',
      motivation: 'Motivation',
      rationale: 'Rationale',
      backwardCompatibility: 'Backward Compatibility',
      referenceImplementation: 'Reference Implementation',
      copyright: 'Copyright/Public Domain'
    },
    type: {
      [ELIP_TYPE.STANDARD_TRACK]: 'The standards tracking ELIP is divided into two parts: design and implementation. The design part should provide basic technical specifications and basic principles as well as implementation guidance. The implementation part is to track the status of the ELIP implementation.',
      [ELIP_TYPE.INFORMATIONAL]: 'Describe the design issues of the Elastos, or provide general guidelines or information to the Elastos community, but does not propose new features. The informational ELIP does not necessarily represent the consensus or recommendation of the Elastos community, so users and implementers are free to ignore Informational ELIP or follow their advice.',
      [ELIP_TYPE.PROCESS]: 'Describes a process surrounding Elastos, or proposes a change to a process. The process ELIP is similar to the Standards ELIP, but it is applicable to areas other than Elastos platform technology. Implementations may also be proposed in the ELIP, but should not involve changes to the Elastos platform codebase.<br />In general, the Process ELIP needs to get community consensus. Unlike the information ELIP, the Process ELIP is more than recommendation, and users are typically not free to ignore them. Examples include procedures, guidelines, changes to the decision-making process, and changes to the tools or environments used in Elastos development. ELIP-1 (this proposal) belongs to the process ELIP.'
    },
    typeTitle: {
      [ELIP_TYPE.STANDARD_TRACK]: 'Standards Tracking ELIP',
      [ELIP_TYPE.INFORMATIONAL]: 'Information ELIP',
      [ELIP_TYPE.PROCESS]: 'Process ELIP'
    },
    error: {
      required: 'This field is required',
      tooLong: 'This field is too long',
      [`limit${ELIP_DESC_MAX_WORDS}`]: `You can only type ${ELIP_DESC_MAX_WORDS} words max.`
    }
  },
  modal: {
    submit: 'Are you sure to submit this ELIP for review?',
    confirm: 'Confirm',
    cancel: 'Cancel',
    reason: 'Reason',
    approve: 'Are you sure to approve this ELIP?',
    abstain: 'Are you sure to abstain this Vote?',
    submittedAsProposal: 'Are you sure to submit this ELIP as Proposal?',
    delete: 'Are you sure to delete this ELIP?'
  },
  text: {
    reviewDetails: 'Review Details',
    approved: 'Approved!',
    rejected: 'Rejected!'
  },
  elip: 'Elip',
  sort: 'Sort by'
}
