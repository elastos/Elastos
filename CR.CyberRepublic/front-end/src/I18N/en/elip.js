import { ELIP_STATUS, ELIP_FILTER } from '@/constant'

export default {
  header: 'ELIPS',
  fields: {
    number: 'Number',
    title: 'Title',
    author: 'Ahthor',
    status: 'Status',
    createdAt: 'Created'
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
    add: 'Add ELIP'
  }
}
