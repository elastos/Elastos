import {
  ELIP_STATUS,
  ELIP_TYPE,
  ELIP_DESC_MAX_WORDS
} from '@/constant'

export default {
  header: 'ELIPS',
  fields: {
    number: '编号',
    title: '标题',
    author: '创建者',
    status: '状态',
    createdAt: '创建日期',
    updatedAt: '修改日期',
    description: '描述',
    type: '类型',
    preamble: '前导',
    abstract: '摘要',
    specification: '规范',
    motivation: '动机',
    rationale: '基本原理',
    backwardCompatibility: '向后兼容性',
    referenceImplementation: '参考实现',
    copyright: '版权/公共域',
    review: '审查',
    vote: '投票',
    preambleItems: {
      elip: 'ELIP',
      title: '标题',
      author: '作者',
      discussions: 'Discussions-To',
      status: '状态',
      type: '类型',
      created: '创建日期',
      requires: 'Requires',
      replaces: 'Replaces',
      superseded: 'Superseded-By'
    },
    filter: '过滤',
    elipStatus: 'Elip 状态',
    creationDate: '创建日期'
  },
  status: {
    [ELIP_STATUS.WAIT_FOR_REVIEW]: '等待审核',
    [ELIP_STATUS.REJECTED]: '已拒绝',
    [ELIP_STATUS.DRAFT]: '草案',
    [ELIP_STATUS.CANCELLED]: '已取消',
    [ELIP_STATUS.SUBMITTED_AS_PROPOSAL]: '提交为提案',
    [ELIP_STATUS.PERSONAL_DRAFT]: '个人草案',
    [ELIP_STATUS.FINAL_REVIEW]: '终审'
  },
  show: '展示',
  button: {
    add: '添加 ELIP',
    cancel: '取消',
    back: '返回',
    submit: '提交',
    reject: '拒绝',
    approve: '批准',
    edit: '编辑',
    submittedAsProposal: '提交为提案',
    continue: '继续',
    yes: '赞成',
    oppose: '反对',
    abstain: '弃权',
    delete: '删除',
    exportAsCSV: '导出CSV文件',
    clearFilter: '重置过滤',
    applyFilter: '应用过滤',
    preview: '预览',
    personalDraft: '保存为个人草案',
    saveChanges: '保存修改',
    cancelled: '取消草案'
  },
  msg: {
    updated: '更新成功',
    submitted: '提交成功',
    rejected: '已驳回',
    approved: '已批准',
    marked: '已标记',
    deleted: '已删除',
    continue: '继续'
  },
  form: {
    note: {
      type: '选择ELIP类型.',
      abstract: '摘要',
      specification: '规范',
      motivation: '动机',
      rationale: '基本原理',
      backwardCompatibility: '向后兼容性',
      referenceImplementation: '参考实现',
      copyright: '版权/公共域',
      review: '审查'
    },
    type: {
      [ELIP_TYPE.STANDARD_TRACK]: '标准跟踪ELIP分为设计和实现两个部分，设计部分应提供基本的技术规范和基本原理以及在实现上的指引，实现部分是对该ELIP实现状态的跟踪。',
      [ELIP_TYPE.INFORMATIONAL]: '描述亦来云的设计问题，或者向亦来云社区提供一般性的指南或信息，但没有提出新的功能。信息ELIP不一定代表亦来云社区的共识或者建议，因此用户和实施者可以自由选择遵循或者忽视。',
      [ELIP_TYPE.PROCESS]: '描述亦来云相关的流程定义或者对流程的改变，流程ELIP类似于标准ELIP，但适用于亦来云平台技术以外的领域。ELIP中也可能提出实施方案，但不应涉及亦来云平台代码库的变更。<br />一般而言，流程类ELIP需要得到社区的共识。与信息类ELIP不同，流程类ELIP不仅仅只是建议，用户通常也不能随意忽略它们。比如：包含规程、指南、决策过程的变化以及对亦来云开发中使用的工具或环境的更改。ELIP-1（本提案）即属于流程ELIP。'
    },
    typeTitle: {
      [ELIP_TYPE.STANDARD_TRACK]: '标准跟踪 ELIP',
      [ELIP_TYPE.INFORMATIONAL]: '信息 ELIP',
      [ELIP_TYPE.PROCESS]: '流程 ELIP'
    },
    error: {
      required: '必填项',
      tooLong: '文字太长',
      [`limit${ELIP_DESC_MAX_WORDS}`]: `不能超过${ELIP_DESC_MAX_WORDS}字`
    }
  },
  modal: {
    submit: '您确定要提交这份 ELIP 吗？',
    confirm: '提交',
    cancel: '取消',
    reason: '理由',
    approve: '您确定要批准该 ELIP 吗？',
    abstain: '您确定要放弃该 Vote 的投票吗？',
    submittedAsProposal: '您确定要提交该 ELIP 作为提案吗？',
    delete: '您确定要删除该 ELIP 吗？'
  },
  text: {
    reviewDetails: '审核详情',
    approved: '已批准！',
    rejected: '已拒绝！'
  },
  elip: 'Elip',
  sort: '排序方式'
}
