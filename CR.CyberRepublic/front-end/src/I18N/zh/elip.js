import { ELIP_STATUS, ELIP_FILTER } from '@/constant'

export default {
  header: 'ELIPS',
  fields: {
    number: '编号',
    title: '标题',
    author: '创建者',
    status: '状态',
    createdAt: '创建日期',
    description: '描述'
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
    add: '添加 ELIP',
    cancel: '取消',
    submit: '提交',
    reject: '拒绝',
    approve: '批准',
    edit: '编辑'
  },
  msg: {
    updated: '更新成功',
    submitted: '提交成功',
    rejected: '已驳回',
    approved: '已批准'
  },
  form: {
    error: {
      required: '必填项',
      tooLong: '文字太长',
      limit3000: '不能超过3000字'
    }
  },
  modal: {
    submit: '您确定要提交这份 ELIP 吗？',
    confirm: '提交',
    cancel: '取消',
    reason: '理由',
    approve: '您确定要批准该 ELIP 吗？'
  },
  note: '是一种设计文档，它用于向亦来云社区提供信息、描述流程、介绍新特性或者环境等。同时，ELIP应该提供目标特性的简明技术规范和基本原理。',
  text: {
    reviewDetails: '审核详情',
    approved: '已批准！',
    rejected: '已拒绝！'
  }
}
