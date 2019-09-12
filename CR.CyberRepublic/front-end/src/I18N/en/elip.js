import {
  ELIP_STATUS,
  ELIP_FILTER,
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
    description: 'Description'
  },
  status: {
    [ELIP_STATUS.WAIT_FOR_REVIEW]: 'WAIT FOR REVIEW',
    [ELIP_STATUS.REJECTED]: 'REJECTED',
    [ELIP_STATUS.DRAFT]: 'DRAFT',
    [ELIP_STATUS.SUBMITTED]: 'SUBMITTED'
  },
  show: 'Show',
  filter: {
    [ELIP_FILTER.ALL]: 'All',
    [ELIP_FILTER.DRAFT]: 'Draft',
    [ELIP_FILTER.WAIT_FOR_REVIEW]: 'Wait for review',
    [ELIP_FILTER.SUBMITTED_BY_ME]: 'Submitted by me'
  },
  button: {
    add: 'Add ELIP',
    cancel: 'Cancel',
    submit: 'Submit for Review',
    reject: 'Reject',
    approve: 'Approve',
    edit: 'Edit',
    markAsSubmitted: 'Mark as Submitted'
  },
  msg: {
    updated: 'Updated successfully',
    submitted: 'Submitted successfully',
    rejected: 'Rejected successfully',
    approved: 'Approved successfully',
    marked: 'Marked successfully'
  },
  form: {
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
    markAsSubmitted: 'Are you sure to mark this ELIP as submitted status?'
  },
  note: 'is a design document that provides information, describes processes, introduces new features, or the environment to the Elastos Community. At the same time, ELIP should provide concise technical specifications and a rationale for the target characteristics.',
  text: {
    reviewDetails: 'Review Details',
    approved: 'Approved!',
    rejected: 'Rejected!'
  }
}
