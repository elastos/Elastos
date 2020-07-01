import {
  ABSTRACT_MAX_WORDS,
  SUGGESTION_STATUS,
  SUGGESTION_BUDGET_TYPE
} from '@/constant'

export default {
  title: {
    add: '添加新建议',
    edit: '编辑建议'
  },
  fields: {
    title: '标题',
    preamble: '前导',
    preambleSub: {
      suggestion: '建议号',
      title: '标题',
      creator: '创建人',
      status: '状态',
      created: '创建日期',
      updated: '更新日期',
      signature: '签名'
    },
    abstract: '摘要',
    goal: '目标',
    motivation: '动机',
    relevance: '关联性',
    budget: '预算',
    type: '类型',
    plan: '实施计划',
    vote: '投票',
    tracking: '跟踪',
    summary: '总结',
    status: '建议状态',
    budgetRequested: '预算',
    creationDate: '创建日期',
    author: '作者'
  },
  btn: {
    makeIntoProposal: '发布为提案',
    needDueDiligence: '需要尽职调查',
    needAdvisory: '需要咨询',
    signSuggetion: '签名建议',
    associateDid: '绑定 DID',
    viewOldData: '查看老数据',
    viewNewData: '查看新数据'
  },
  status: {
    posted: '发布',
    underConsideration: '委员会正在审议中',
    moreInfoRequired: '需要更多信息',
    referred: '相关提案',
    [SUGGESTION_STATUS.ACTIVE]: '活跃',
    [SUGGESTION_STATUS.ABUSED]: '已举报',
    [SUGGESTION_STATUS.ARCHIVED]: '已归档'
  },
  form: {
    search: '搜索建议',
    button: {
      continue: '继续',
      cancel: '取消',
      saveDraft: '保存草稿',
      save: '保存并发布',
      update: '更新',
      create: '创建',
      discardChanges: '放弃修改',
      revertVersion: '使用该版本',
      showVersion: '显示历史版本'
    },
    fields: {
      title: '标题'
    },
    type: {
      newMotion: '新动议',
      motionAgainst: '反对动议',
      anythingElse: '其它事宜'
    },
    note: {
      type: '选择一个建议类型。',
      abstract: '一个关于提案内容的简短描述（不超过200字）。',
      goal: '描述通过执行提案期望达到的效果。目标应该是清晰且可度量的。',
      motivation: '描述为什么会提出这个提案。对于试图对亦来云有所改变的提案来说，动机至关重要。这里应该清楚的解释为什么现有的机制不足以解决提案想解决的问题，',
      motivationHighlight: '没有足够动机的提案被拒的可能性很大。',
      relevance: '如果和其它CRC提案有所关联，这里应该提供关联提案的提案号并且说明和相关提案的关系。如果与亦来云技术或者其它CRC提案有冲突，则应该对这些冲突进行说明并解释怎么处理它们。',
      budget: '如果执行提案需要CRC的经费支持，这里应该说明总的预算以及支出计划。这是一个和执行计划配套的财务计划。',
      type: '选择一个提案类型。',
      plan: '这里应该说明通过什么方法和过程达成目标，对执行人或者团队应该有一个简单的介绍。如果提案的执行周期比较长，应该设立一些执行过程中的检查点，两个检查点之间不超过3个月。和提案目标一样，检查点也应该是清晰且可度量的。',
      tracking: '当提案完成的时候，提案人应该在这里提交对提案执行状况的一个总结，包括目标达成状况和财务决算。CRC秘书处负责该部分内容的审核。'
    },
    error: {
      required: '必填项',
      tooLong: '文字太长',
      [`limit${ABSTRACT_MAX_WORDS}`]: `不能超过${ABSTRACT_MAX_WORDS}字`,
      isNaN: '请输入有效的金额',
      plan: '实施团队或里程碑为空',
      milestones: '里程碑为空',
      team: '实施团队为空',
      amount: '金额大于总金额的30%',
      previousMilestoneDate: '日期必须晚于前一个里程碑',
      requirePayment: '在完成提案之前，需要填写项目的支付项。',
      elaAddress: 'ELA 地址无效',
      schedule: '支付计划为空',
      payment: '支付项和里程碑必须保持一致，必须有结项款，至多一项预付款',
      advance: '预付款只能用于项目第一阶段',
      completion: '结项款只能用于项目最后阶段.',
      conditioned: '该阶段支付不能用于项目的这个阶段',
      isUsed: '该项目阶段已被其它支付项使用了',
      exception: '出错了',
      notEqual: '支付项款之和与项目总金额不一致'
    }
  },
  modal: {
    addTagComment: '添加评论',
    confirm: '确定',
    cancel: '取消',
    pleaseUpdate: '请按照需求更新建议，并通过评论通知委员或社区',
    commentsFromCouncil: '来自委员或管理员的评论:',
    consideration: '您确定要将此建议标记为委员会正在审议中？',
    signNotice: '您的建议签名之后，才能被委员审核。您不能修改签名后的建议。',
    signNow: '签名',
    signLater: '稍后'
  },
  tag: {
    show: '展示',
    type: {
      UNDER_CONSIDERATION: '委员会正在审议中',
      INFO_NEEDED: '需要更多信息',
      ADDED_TO_PROPOSAL: '已添加到提案'
    }
  },
  msg: {
    consideration: '已标记为委员会正在审议中',
    notify: '已给秘书发送邮件',
    archived: '建议已归档',
    unarchived: '建议已取消归档',
    revertVersion: '已使用指定版本的内容',
    madeByOtherCM: '其他委员已经把这个建议转成提案了。',
    councilQRCode: '扫描上面二维码把建议转成提案，提案上链后请到网站提案列表页面查看',
    toChain: '正在上链中',
    signQRCode: '扫描上面二维码签名建议',
    associateDidFirst: '请先绑定您的 DID'
  },
  header: {
    suggestion: '建议',
    crCouncil: 'CR 委员会',
    crProposalPage: 'CR 提案页面',
    approvedProposal: '已批准的提案'
  },
  search: {
    number: '编号',
    title: '标题',
    abstract: '摘要',
    email: '作者邮箱',
    name: '作者姓名'
  },
  budget: {
    create: '创建支付项',
    payment: '支付',
    type: '类型',
    amount: '金额',
    reasons: '理由',
    criteria: '支付标准',
    action: '操作',
    desc: '请基于里程碑填写：',
    schedule: '支付计划',
    total: '总金额',
    address: 'ELA 接收地址',
    tip: '请先添加里程碑',
    [SUGGESTION_BUDGET_TYPE.ADVANCE]: '预支付',
    [SUGGESTION_BUDGET_TYPE.COMPLETION]: '结项款',
    [SUGGESTION_BUDGET_TYPE.CONDITIONED]: '阶段支付',
    goal: '目标',
    milestone: '里程碑'
  },
  plan: {
    teamMember: '团队成员',
    role: '角色',
    responsibility: '职责',
    moreInfo: '更多信息',
    createTeamInfo: '添加团队成员',
    action: '操作',
    teamInfo: '实施团队',
    milestones: '里程碑',
    publishDate: '发布日期',
    version: '版本',
    goal: '目标',
    showDetail: '显示详细信息',
    hideDetail: '隐藏详细信息',
    selectDate: '选择日期',
    milestone: '里程碑'
  },
  label: {
    hasMadeIntoProposal: '已经将此建议发布为'
  }
}
