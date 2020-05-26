import {
  MILESTONE_STATUS
} from '@/constant'
const {
  WAITING_FOR_APPROVAL,
  WAITING_FOR_REQUEST,
  WAITING_FOR_WITHDRAWAL,
  REJECTED,
  WITHDRAWN
} = MILESTONE_STATUS

export default {
  status: '状态',
  request: '申请',
  rerequest: '重新申请',
  reject: '拒绝',
  approve: '同意',
  withdraw: '取款',
  [WAITING_FOR_REQUEST]: '等待申请',
  [WAITING_FOR_APPROVAL]: '等待审核',
  [WAITING_FOR_WITHDRAWAL]: '等待取款',
  [WITHDRAWN]: '已取款',
  [REJECTED]: '已拒绝'
}
