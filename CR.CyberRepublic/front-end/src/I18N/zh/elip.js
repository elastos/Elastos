import {
  ELIP_STATUS,
  ELIP_FILTER,
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
    description: '描述',
    type: '类型',
    abstract: '摘要',
    specifications: '规范',
    motivation: '动机',
    rationale: '基本原理',
    backwardCompatibility: '向后兼容性',
    referenceImplementation: '参考实现',
    copyrightDomain: '版权/公共域',
    review: 'Review',
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
    }
  },
  status: {
    [ELIP_STATUS.WAIT_FOR_REVIEW]: '等待审核',
    [ELIP_STATUS.REJECTED]: '已拒绝',
    [ELIP_STATUS.DRAFT]: '草案',
    [ELIP_STATUS.SUBMITTED]: '已提交'
  },
  show: '展示',
  filter: {
    [ELIP_FILTER.ALL]: '全部',
    [ELIP_FILTER.DRAFT]: '草案',
    [ELIP_FILTER.WAIT_FOR_REVIEW]: '等待审核',
    [ELIP_FILTER.SUBMITTED_BY_ME]: '我提交的'
  },
  button: {
    add: '添加 ELIP',
    cancel: '取消',
    submit: '提交',
    reject: '拒绝',
    approve: '批准',
    edit: '编辑',
    markAsSubmitted: '标记为提交状态',
    continue: '继续'
  },
  msg: {
    updated: '更新成功',
    submitted: '提交成功',
    rejected: '已驳回',
    approved: '已批准',
    marked: '已标记'
  },
  form: {
    note: {
      type: '选择ELIP类型.',
      abstract: 'Abstract',
      specifications: 'Specifications',
      motivation: 'Motivation',
      rationale: 'Rationale',
      backwardCompatibility: 'Backward Compatibility',
      referenceImplementation: 'Reference Implementation',
      copyrightDomain: 'Copyright/Public Domain',
      review: 'Review'
    },
    type: {
      standardTitle: 'Standards Tracking ELIP',
      informationTitle: 'Information ELIP',
      processTitle: 'Process ELIP',
      standard: 'The standards tracking ELIP is divided into two parts: design and implementation. The design part should provide basic technical specifications and basic principles as well as implementation guidance. The implementation part is to track the status of the ELIP implementation.',
      information: 'Describe the design issues of the Elastos, or provide general guidelines or information to the Elastos community, but does not propose new features. The informational ELIP does not necessarily represent the consensus or recommendation of the Elastos community, so users and implementers are free to ignore Informational ELIP or follow their advice.',
      process: 'Describes a process surrounding Elastos, or proposes a change to a process. The process ELIP is similar to the Standards ELIP, but it is applicable to areas other than Elastos platform technology. Implementations may also be proposed in the ELIP, but should not involve changes to the Elastos platform codebase.<br />In general, the Process ELIP needs to get community consensus. Unlike the information ELIP, the Process ELIP is more than recommendation, and users are typically not free to ignore them. Examples include procedures, guidelines, changes to the decision-making process, and changes to the tools or environments used in Elastos development. ELIP-1 (this proposal) belongs to the process ELIP.',
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
    markAsSubmitted: '您确定要把该 ELIP 标记为提交状态吗？',
    markAsCancelled: '您确定要把该 ELIP 标记为取消状态吗？'
  },
  text: {
    reviewDetails: '审核详情',
    approved: '已批准！',
    rejected: '已拒绝！'
  }
}
