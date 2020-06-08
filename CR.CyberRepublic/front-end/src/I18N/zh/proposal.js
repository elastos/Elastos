import {
  CVOTE_TRACKING_STATUS,
  CVOTE_SUMMARY_STATUS,
  ABSTRACT_MAX_WORDS
} from '@/constant'

export default {
  fields: {
    title: '标题',
    preamble: '前导',
    preambleSub: {
      proposal: 'CRC提案号',
      title: '标题',
      proposer: '提案人',
      referee: '推荐委员',
      status: '状态',
      created: '创建日期'
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
    status: '提案状态',
    budgetRequested: '预算',
    hasTrackingMsg: '是否有跟踪信息需要审核',
    isUnvotedByYou: '未被自己投票的提案',
    creationDate: '创建日期',
    author: '作者',
    endsDate: '结束日期'
  },
  btn: {
    tracking: {
      reject: '驳回 & 评论',
      approve: '批准 & 发布'
    },
    summary: {
      reject: '驳回 & 评论',
      approve: '批准 & 发布'
    },
    viewOldData: '查看老数据',
    viewNewData: '查看新数据'
  },
  text: {
    tracking: {
      reviewDetails: '审阅详情',
      notice: {
        header: '',
        footer: ''
      }
    },
    summary: {
      reviewDetails: '审阅详情',
      notice: {
        header: '',
        footer: ''
      }
    }
  },
  status: {
    tracking: {
      [CVOTE_TRACKING_STATUS.DRAFT]: '草稿',
      [CVOTE_TRACKING_STATUS.REVIEWING]: '审阅中',
      [CVOTE_TRACKING_STATUS.PUBLISHED]: '已批准 ✓',
      [CVOTE_TRACKING_STATUS.REJECT]: '被驳回 !'
    },
    summary: {
      [CVOTE_SUMMARY_STATUS.DRAFT]: '草稿',
      [CVOTE_SUMMARY_STATUS.REVIEWING]: '审阅中',
      [CVOTE_SUMMARY_STATUS.PUBLISHED]: '已批准 ✓',
      [CVOTE_SUMMARY_STATUS.REJECT]: '被驳回 !'
    },
    trackingRaw: {
      undefined: '',
      [CVOTE_TRACKING_STATUS.DRAFT]: '草稿',
      [CVOTE_TRACKING_STATUS.REVIEWING]: '审阅中',
      [CVOTE_TRACKING_STATUS.PUBLISHED]: '已批准',
      [CVOTE_TRACKING_STATUS.REJECT]: '被驳回'
    },
    summaryRaw: {
      undefined: '',
      [CVOTE_SUMMARY_STATUS.DRAFT]: '草稿',
      [CVOTE_SUMMARY_STATUS.REVIEWING]: '审阅中',
      [CVOTE_SUMMARY_STATUS.PUBLISHED]: '已批准',
      [CVOTE_SUMMARY_STATUS.REJECT]: '被驳回'
    }
  },
  form: {
    tracking: {
      btn: {
        submit: '提交'
      },
      add: '添加进度'
    },
    summary: {
      btn: {
        submit: '提交'
      },
      add: '添加总结'
    },
    note: {
      abstract: '一个关于提案内容的简短描述（不超过200字）。',
      goal: '描述通过执行提案期望达到的效果。目标应该是清晰且可度量的。',
      motivation: '描述为什么会提出这个提案。对于试图对亦来云有所改变的提案来说，动机至关重要。这里应该清楚的解释为什么现有的机制不足以解决提案想解决的问题，',
      motivationHighlight: '没有足够动机的提案被拒的可能性很大。',
      relevance: '如果和其它CRC提案有所关联，这里应该提供关联提案的提案号并且说明和相关提案的关系。如果与亦来云技术或者其它CRC提案有冲突，则应该对这些冲突进行说明并解释怎么处理它们。',
      budget: '如果执行提案需要CRC的经费支持，这里应该说明总的预算以及支出计划。这是一个和执行计划配套的财务计划。',
      type: '选择一个提案类型。',
      plan: '这里应该说明通过什么方法和过程达成目标，对执行人或者团队应该有一个简单的介绍。如果提案的执行周期比较长，应该设立一些执行过程中的检查点，两个检查点之间不超过3个月。和提案目标一样，检查点也应该是清晰且可度量的。',
      tracking: '这个部分由提案人负责按提案进度修改，内容包括目标完成情况及经费使用情况，用于按照执行计划中的检查点或提案目标跟踪提案的执行状况。CRC秘书处负责该部分内容的跟踪和审核。',
      summary: '当提案完成的时候，提案人应该在这里提交对提案执行状况的一个总结，包括目标达成状况和财务决算。CRC秘书处负责该部分内容的审核。'
    },
    error: {
      required: '必填项',
      tooLong: '文字太长',
      [`limit${ABSTRACT_MAX_WORDS}`]: '不能超过200字'
    },

  },
  msg: {
    rejected: '已驳回',
    approved: '已批准并发布',
    draftSaved: '草稿保存成功，请前往提案列表查看',
    proposalPublished: '提案发布成功'
  }
}
