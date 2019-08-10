import { CVOTE_TRACKING_STATUS, CVOTE_SUMMARY_STATUS } from '@/constant'

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
    summary: '总结'
  },
  btn: {
    tracking: {
      reject: 'Reject & Comment',
      approve: 'Approve & Publish'
    },
    summary: {
      reject: 'Reject & Comment',
      approve: 'Approve & Publish'
    },
    makeIntoProposal: '发布为提案'
  },
  text: {
    tracking: {
      reviewDetails: 'Review Details',
      notice: {
        header: '',
        footer: ''
      }
    },
    summary: {
      reviewDetails: 'Review Details',
      notice: {
        header: '',
        footer: ''
      }
    }
  },
  status: {
    tracking: {
      [CVOTE_TRACKING_STATUS.DRAFT]: 'DRAFT',
      [CVOTE_TRACKING_STATUS.REVIEWING]: 'BEING REVIEWED',
      [CVOTE_TRACKING_STATUS.PUBLISHED]: 'APPROVED ✓',
      [CVOTE_TRACKING_STATUS.REJECT]: 'REJECTED !'
    },
    summary: {
      [CVOTE_SUMMARY_STATUS.DRAFT]: 'DRAFT',
      [CVOTE_SUMMARY_STATUS.REVIEWING]: 'BEING REVIEWED',
      [CVOTE_SUMMARY_STATUS.PUBLISHED]: 'APPROVED ✓',
      [CVOTE_SUMMARY_STATUS.REJECT]: 'REJECTED !'
    },
    trackingRaw: {
      undefined: '',
      [CVOTE_TRACKING_STATUS.DRAFT]: 'DRAFT',
      [CVOTE_TRACKING_STATUS.REVIEWING]: 'BEING REVIEWED',
      [CVOTE_TRACKING_STATUS.PUBLISHED]: 'APPROVED',
      [CVOTE_TRACKING_STATUS.REJECT]: 'REJECTED'
    },
    summaryRaw: {
      undefined: '',
      [CVOTE_SUMMARY_STATUS.DRAFT]: 'DRAFT',
      [CVOTE_SUMMARY_STATUS.REVIEWING]: 'BEING REVIEWED',
      [CVOTE_SUMMARY_STATUS.PUBLISHED]: 'APPROVED',
      [CVOTE_SUMMARY_STATUS.REJECT]: 'REJECTED'
    },
    posted: '发布',
    underConsideration: '正在审议中',
    moreInfoRequired: '需要更多信息',
    referred: '相关提案'
  },
  form: {
    button: {
      continue: '继续',
      cancel: '取消',
      saveDraft: '保存草稿',
      save: '保存并发布'
    },
    tracking: {
      btn: {
        submit: 'Submit for Review'
      },
      add: 'Add Tracking Plan'
    },
    summary: {
      btn: {
        submit: 'Submit for Review'
      },
      add: 'Add Summary'
    },
    fields: {
      title: '标题',
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
      motivation:
        '描述为什么会提出这个提案。对于试图对亦来云有所改变的提案来说，动机至关重要。这里应该清楚的解释为什么现有的机制不足以解决提案想解决的问题，',
      motivationHighlight: '没有足够动机的提案被拒的可能性很大。',
      relevance:
        '如果和其它CRC提案有所关联，这里应该提供关联提案的提案号并且说明和相关提案的关系。如果与亦来云技术或者其它CRC提案有冲突，则应该对这些冲突进行说明并解释怎么处理它们。',
      budget:
        '如果执行提案需要CRC的经费支持，这里应该说明总的预算以及支出计划。这是一个和执行计划配套的财务计划。',
      type: '选择一个提案类型。',
      plan:
        '这里应该说明通过什么方法和过程达成目标，对执行人或者团队应该有一个简单的介绍。如果提案的执行周期比较长，应该设立一些执行过程中的检查点，两个检查点之间不超过3个月。和提案目标一样，检查点也应该是清晰且可度量的。',
      tracking:
        '这个部分由提案人负责按提案进度修改，内容包括目标完成情况及经费使用情况，用于按照执行计划中的检查点或提案目标跟踪提案的执行状况。CRC秘书处负责该部分内容的跟踪和审核。',
      summary:
        '当提案完成的时候，提案人应该在这里提交对提案执行状况的一个总结，包括目标达成状况和财务决算。CRC秘书处负责该部分内容的审核。'
    },
    error: {
      required: '必填项',
      tooLong: '文字太长',
      limit200: '不能超过200字'
    }
  },
  msg: {
    rejected: 'Rejected successfully',
    approved: 'Approved and published successfully',
    draftSaved:
      'Saved as draft successfully, you can check it in proposal list page',
    proposalPublished: 'Proposal published successfully'
  },
  modal: {
    addTagComment: 'Add Comment',
    confirm: 'Confirm',
    cancel: 'Cancel',
    pleaseUpdate:
      'Please update the Suggestion with the requested info and notify the council/community through a comment.',
    commentsFromCouncil: 'Comments from Council/Admin:',
    consideration: '标记为委员会正在审议中'
  },
  tag: {
    show: 'Show',
    type: {
      UNDER_CONSIDERATION: 'Under Council Consideration',
      INFO_NEEDED: 'More Info Required',
      ADDED_TO_PROPOSAL: 'Added to Proposal'
    }
  }
}
