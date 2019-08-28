import { ELIP_STATUS, ELIP_FILTER } from '@/constant'

export default {
  header: 'ELIPS',
  fields: {
    number: 'Number',
    title: 'Title',
    author: 'Ahthor',
    status: 'Status',
    createdAt: 'Created',
    description: 'Description'
  },
  status: {
    [ELIP_STATUS.WAIT_FOR_REVIEW]: 'WAIT FOR REVIEW',
    [ELIP_STATUS.REJECTED]: 'REJECTED',
    [ELIP_STATUS.APPROVED]: 'APPROVED'
  },
  show: 'Show',
  filter: {
    [ELIP_FILTER.ALL]: 'All',
    [ELIP_FILTER.APPROVED]: 'Approved',
    [ELIP_FILTER.WAIT_FOR_REVIEW]: 'Wait for review',
    [ELIP_FILTER.SUBMITTED_BY_ME]: 'Submitted by me'
  },
  button: {
    add: 'Add ELIP',
    cancel: 'Cancel',
    submit: 'Submit for Review',
    reject: 'Reject',
    approve: 'Approve',
    edit: 'Edit'
  },
  msg: {
    updated: 'Updated successfully',
    submitted: 'Submitted successfully',
    rejected: 'Rejected successfully',
    approved: 'Approved successfully'
  },
  form: {
    error: {
      required: 'This field is required',
      tooLong: 'This field is too long',
      limit3000: 'You can only type 3000 words max.'
    }
  },
  modal: {
    submit: 'Are you sure to submit this ELIP for review?',
    confirm: 'Confirm',
    cancel: 'Cancel',
    reason: 'Reason',
    approve: 'Are you sure to approve this ELIP?'
  },
  note: 'is a design document that provides information, describes processes, introduces new features, or the environment to the Elastos Community. At the same time, ELIP should provide concise technical specifications and a rationale for the target characteristics.',
  text: {
    reviewDetails: 'Review Details',
    approved: 'Approved!',
    rejected: 'Rejected!'
  }
}
