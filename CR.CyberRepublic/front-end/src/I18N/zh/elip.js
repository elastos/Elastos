import { ELIP_STATUS, ELIP_FILTER } from '@/constant'

export default {
  header: 'ELIPS',
  fields: {
    number: '编号',
    title: '标题',
    author: '创建者',
    status: '状态',
    createdAt: '创建日期'
  },
  status: {
    [ELIP_STATUS.WAIT_FOR_REVIEW]: '等待审核',
    [ELIP_STATUS.REJECTED]: '已拒绝',
    [ELIP_STATUS.APPROVED]: '已批准'
  },
  show: '展示',
  filter: {
    [ELIP_FILTER.ALL]: '全部',
    [ELIP_FILTER.APPROVED]: '已批准',
    [ELIP_FILTER.WAIT_FOR_REVIEW]: '等待审核',
    [ELIP_FILTER.SUBMITTED_BY_ME]: '已提交的'
  },
  button: {
    add: '添加 ELIP'
  }
}
