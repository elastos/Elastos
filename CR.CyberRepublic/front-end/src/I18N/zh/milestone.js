import { MILESTONE_STATUS } from '@/constant'
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
  [REJECTED]: '已拒绝',
  sign: '扫码上面二维码签名',
  summary: '总结',
  reason: '理由',
  exception: '出错了',
  header: '申请款项',
  scanToWithdraw: '扫码上面二维码取款',
  required: '必填项',
  next: '下一步',
  noUtxos: '提款业务繁忙，请稍后再试'
}
