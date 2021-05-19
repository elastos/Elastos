import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Form,
  Icon,
  Input,
  InputNumber,
  DatePicker,
  Button,
  Checkbox,
  Select,
  Popover,
  Row,
  Col,
  Upload,
  Cascader,
  message,
  Popconfirm,
  TreeSelect,
  Modal,
  Switch,
  Divider,
  Card
} from 'antd'
import CodeMirrorEditor from '@/module/common/CodeMirrorEditor'

import I18N from '@/I18N'
import { upload_file } from '@/util'
import './style.scss'
import moment from 'moment'
import _ from 'lodash'
import { getSafeUrl } from '@/util/url'

import {
  TEAM_TASK_DOMAIN,
  SKILLSET_TYPE,
  TASK_CATEGORY,
  TASK_TYPE,
  TASK_STATUS,
  TASK_EVENT_DATE_TYPE
} from '@/constant'

const FormItem = Form.Item
const TextArea = Input.TextArea
const Option = Select.Option

/**
 * This is generic task create form for both Developer and Social Bounties / Events
 *
 * Which version of the form depends on the leader's program
 *
 * Leaders - can create:
 * - Events (offline) restricted to their area - must be approved
 * - Events (online) anywhere - Social or Developer
 *
 * TODO: in the future we should developer leaders
 * TODO: this form is getting long, maybe it should be a 2-3 step form?
 *
 * Community Leaders - each community has a leader
 * - a leader can create events in their own local community or online community
 * - local offline events are automatically shown in their local community, a country leader
 *  can create events in any child community
 * - these events are shown in the Social page as well
 * - a local event can have sub tasks, these are shown as tasks in the Social page
 */
class C extends BaseComponent {
  async componentDidMount() {
    const taskId = this.props.match.params.taskId
    taskId && (await this.props.getTaskDetail(taskId))
    this.props.getAllCircles()
  }

  componentWillUnmount() {
    this.props.resetTaskDetail()
  }

  handleSubmit(e) {
    e.preventDefault()
    this.props.form.validateFields((err, values) => {
      if (!err) {
        if (values.taskCommunity) {
          if (values.taskCommunity.length > 1) {
            values.communityParent = values.taskCommunity[0]
            values.community = values.taskCommunity[1]
          } else {
            values.communityParent = null
            values.community = values.taskCommunity[0]
          }
        }

        // admin does not need to agree to disclaimer
        if (!this.props.is_admin) {
          if (
            values.taskReward ||
            values.taskRewardUsd ||
            values.taskRewardUpfront ||
            values.taskRewardUpfrontUsd
          ) {
            if (!this.state.readDisclaimer) {
              message.error(I18N.get('from.TaskCreateForm.message.error'))
              document.getElementById('disclaimerLink').focus()
              return
            }

            values.readDisclaimer = true
          }
        }

        values.pictures = this.state.fileList || []
        _.each(values.pictures, pictureFile => {
          if (this.pictureUrlLookups[pictureFile.uid]) {
            pictureFile.url = this.pictureUrlLookups[pictureFile.uid]
          }
        })

        values.bidding = this.state.isBidding
        values.assignSelf = this.state.assignSelf

        if (this.props.is_project) {
          values.type = TASK_TYPE.PROJECT
        }

        values.pitch = values.pitch || {}
        _.each(
          [
            'problem',
            'valueProposition',
            'useCase',
            'beneficiaries',
            'elaInfrastructure'
          ],
          pitchKeyword => {
            values.pitch[pitchKeyword] = values[pitchKeyword]
            delete values[pitchKeyword]
          }
        )

        if (this.state.editing) {
          this.props.updateTask(values, this.state).then(() => {
            this.props.getTaskDetail(this.props.existingTask._id)
          })
          this.props.switchEditMode()
        } else {
          this.props.createTask(values, this.state)
        }
      }
    })
  }

  constructor(props) {
    super(props)

    this.state = {
      communityTrees: [],
      taskType: this.props.taskType || TASK_TYPE.TASK,
      taskCategory: this.props.taskCategory || TASK_CATEGORY.GENERAL,
      assignSelf: props.existingTask
        ? props.existingTask.assignSelf
        : !this.props.circleId,
      eventDateRange:
        (props.existingTask && props.existingTask.eventDateRange) || false,
      thumbnail_url:
        (props.existingTask && props.existingTask.thumbnail) || null,
      thumbnail_loading: false,
      thumbnail_filename:
        (props.existingTask && props.existingTask.thumbnailFilename) || '',
      thumbnail_type: '',
      attachment_url:
        (props.existingTask && props.existingTask.attachment) || null,
      attachment_loading: false,
      attachment_filename:
        (props.existingTask && props.existingTask.attachmentFilename) || '',
      attachment_type: '',
      circle:
        (props.existingTask &&
          props.existingTask.circle &&
          props.existingTask.circle._id) ||
        props.circleId ||
        null,
      removeAttachment: false,
      editing: !!props.existingTask,
      fileList: (props.existingTask && props.existingTask.pictures) || [],
      previewVisible: false,
      previewImage: '',
      isBidding: (props.existingTask && props.existingTask.bidding) || false,
      readDisclaimer:
        (props.existingTask && props.existingTask.readDisclaimer) || false,
      showDisclaimer: false
    }

    this.pictureUrlLookups = []
    _.each(this.state.fileList, file => {
      this.pictureUrlLookups[file.uid] = file.url
    })
  }

  hasLeaderEditRestrictions() {
    const existingTask = this.props.existingTask
    return (
      this.props.page === 'LEADER' &&
      ![TASK_STATUS.CREATED, TASK_STATUS.PENDING].includes(existingTask.status)
    )
  }

  getInputProps() {
    const { getFieldDecorator } = this.props.form
    const existingTask = this.props.existingTask

    // if this is approved - TODO: back-end restrictions + tests
    // this is for the organizer page and editing is more restrictive
    // unless it's just CREATED/PENDING
    const hasLeaderEditRestrictions = this.hasLeaderEditRestrictions()

    const taskName_fn = getFieldDecorator('taskName', {
      rules: [
        {
          required: true,
          message: I18N.get('from.TaskCreateForm.taskName.required')
        },
        { min: 4, message: I18N.get('from.TaskCreateForm.taskName.min') }
      ],
      initialValue: this.state.editing && existingTask ? existingTask.name : ''
    })
    const taskName_el = <Input size="large" />

    const taskCategory_fn = getFieldDecorator('taskCategory', {
      rules: [
        {
          required: true,
          message: I18N.get('from.TaskCreateForm.taskCategory.required')
        }
      ],
      initialValue: this.state.editing
        ? existingTask.category
        : this.state.taskCategory || TASK_CATEGORY.SOCIAL
    })
    const taskCategory_el = (
      <Select
        disabled={hasLeaderEditRestrictions}
        onChange={val => {
          this.setState({ taskCategory: val })
          if (this.state.taskCategory === TASK_TYPE.PROJECT) {
            // this.setState({taskType: TASK_TYPE.TASK})
          }
        }}
      >
        <Option value={TASK_CATEGORY.GENERAL}>General</Option>
        <Option value={TASK_CATEGORY.SOCIAL}>Social</Option>
        <Option value={TASK_CATEGORY.DEVELOPER}>Developer</Option>
      </Select>
    )

    const circle_fn = getFieldDecorator('circle', {
      rules: [],
      initialValue: this.props.all_circles_loading
        ? I18N.get('.loading')
        : (!this.props.loading &&
            this.state.editing &&
            existingTask.circle &&
            existingTask.circle._id) ||
          this.props.circleId ||
          null
    })

    const circle_el = (
      <Select
        disabled={
          this.props.all_circles_loading || this.props.disableCircleSelect
        }
      >
        <Select.Option value={null}>{I18N.get('.no')}</Select.Option>
        {_.map(this.props.all_circles, (circle, ind) => (
          <Select.Option key={ind} value={circle._id}>
            {circle.name}
          </Select.Option>
        ))}
      </Select>
    )

    // sub-tasks are not here because those can only be created from an existing Task Detail Page
    const taskType_fn = getFieldDecorator('taskType', {
      rules: [
        {
          required: true,
          message: I18N.get('from.TaskCreateForm.taskType.required')
        }
      ],
      initialValue: this.state.editing
        ? existingTask.type
        : this.state.taskType || TASK_TYPE.TASK
    })
    const taskType_el = (
      <Select
        disabled={hasLeaderEditRestrictions}
        onChange={val => this.setState({ taskType: val })}
      >
        <Option value={TASK_TYPE.TASK}>
          {I18N.get('from.TaskCreateForm.taskType.option.task')}
        </Option>
        {this.state.taskCategory === TASK_CATEGORY.DEVELOPER &&
          (this.props.is_admin || this.props.is_leader) && (
            <Option value={TASK_TYPE.PROJECT}>
              {I18N.get('from.TaskCreateForm.taskType.option.project')}
            </Option>
          )}
        <Option value={TASK_TYPE.EVENT}>
          {I18N.get('from.TaskCreateForm.taskType.option.event')}
        </Option>
      </Select>
    )

    // TODO: restrict community to only the one you are in
    const taskCommunity_fn = getFieldDecorator('taskCommunity', {
      initialValue: existingTask ? existingTask.taskCommunity : []
    })
    const taskCommunity_el = (
      <Cascader
        options={this.state.communityTrees}
        placeholder=""
        changeOnSelect={true}
      />
    )

    const applicationDeadline_fn = getFieldDecorator(
      'taskApplicationDeadline',
      {
        initialValue:
          this.state.editing &&
          existingTask.applicationDeadline &&
          moment(existingTask.applicationDeadline).isValid()
            ? moment(existingTask.applicationDeadline)
            : null
      }
    )
    const applicationDeadline_el = <DatePicker />

    const completionDeadline_fn = getFieldDecorator('taskCompletionDeadline', {
      initialValue:
        this.state.editing &&
        existingTask.completionDeadline &&
        moment(existingTask.completionDeadline).isValid()
          ? moment(existingTask.completionDeadline)
          : null
    })
    const completionDeadline_el = <DatePicker />

    const description = this.state.editing ? existingTask.description : ''
    const taskDesc_fn = getFieldDecorator('taskDesc', {
      rules: [
        {
          required: true,
          message: I18N.get('from.TaskCreateForm.taskDesc.required')
        },
        { max: 8192, message: I18N.get('from.TaskCreateForm.taskDesc.max') }
      ],
      initialValue: description
    })
    const taskDesc_el = (
      <CodeMirrorEditor content={description} name="taskDesc" />
    )
    const descBreakdown = this.state.editing ? existingTask.descBreakdown : ''
    const taskDescBreakdown_fn = getFieldDecorator('taskDescBreakdown', {
      rules: [
        {
          max: 8192,
          message: I18N.get('from.TaskCreateForm.taskDescBreakdown.max')
        }
      ],
      initialValue: descBreakdown
    })
    const taskDescBreakdown_el = (
      <CodeMirrorEditor content={descBreakdown} name="taskDescBreakdown" />
    )
    const taskGoals_fn = getFieldDecorator('taskGoals', {
      rules: [
        { max: 8192, message: I18N.get('from.TaskCreateForm.taskGoals.max') }
      ],
      initialValue: this.state.editing ? existingTask.goals : ''
    })
    const taskGoals_el = <TextArea rows={4} />

    const taskLink_fn = getFieldDecorator('taskLink', {
      rules: [
        {
          required: false,
          message: I18N.get('from.TaskCreateForm.taskLink.required')
        }
      ],
      initialValue: this.state.editing ? existingTask.infoLink : ''
    })
    const taskLink_el = <Input size="large" />

    /*
     ********************************************************************************************************
     * Event Info
     ********************************************************************************************************
     */
    const eventDateRange_fn = getFieldDecorator('eventDateRange')
    const eventDateRange_el = (
      <Checkbox
        checked={this.state.eventDateRange}
        onClick={() =>
          this.setState({ eventDateRange: !this.state.eventDateRange })
        }
      />
    )

    const eventDateStatus_fn = getFieldDecorator('eventDateStatus', {
      initialValue:
        this.state.editing && existingTask.eventDateStatus
          ? existingTask.eventDateStatus
          : TASK_EVENT_DATE_TYPE.TENTATIVE
    })
    const eventDateStatus_el = (
      <Select>
        <Option value={TASK_EVENT_DATE_TYPE.NOT_APPLICABLE}>N/A</Option>
        <Option value={TASK_EVENT_DATE_TYPE.TENTATIVE}>Tentative</Option>
        <Option value={TASK_EVENT_DATE_TYPE.CONFIRMED}>Confirmed</Option>
      </Select>
    )

    const eventDateRangeStart_fn = getFieldDecorator('eventDateRangeStart', {
      initialValue:
        this.state.editing &&
        existingTask.eventDateRangeStart &&
        moment(existingTask.eventDateRangeStart).isValid()
          ? moment(existingTask.eventDateRangeStart)
          : null
    })
    const eventDateRangeStart_el = (
      <DatePicker showTime={true} format="YYYY-MM-DD HH:mm" />
    )

    const eventDateRangeEnd_fn = getFieldDecorator('eventDateRangeEnd', {
      initialValue:
        this.state.editing &&
        existingTask.eventDateRangeEnd &&
        moment(existingTask.eventDateRangeEnd).isValid()
          ? moment(existingTask.eventDateRangeEnd)
          : null
    })
    const eventDateRangeEnd_el = (
      <DatePicker showTime={true} format="YYYY-MM-DD HH:mm" />
    )

    const taskLocation_fn = getFieldDecorator('taskLocation', {
      rules: [
        {
          required: false,
          message: I18N.get('from.TaskCreateForm.taskLocation.required')
        }
      ],
      initialValue: this.state.editing ? existingTask.location : ''
    })
    const taskLocation_el = <Input size="large" />

    /*
     ********************************************************************************************************
     * Budget / Reward
     ********************************************************************************************************
     */

    const taskCandLimit_fn = getFieldDecorator('taskCandLimit', {
      rules: [
        {
          required: true,
          type: 'integer',
          message: I18N.get('from.TaskCreateForm.taskCandLimit.required')
        }
      ],
      initialValue: this.state.editing ? existingTask.candidateLimit : 10
    })
    const taskCandLimit_el = (
      <InputNumber size="large" disabled={hasLeaderEditRestrictions} />
    )

    const taskCandSltLimit_fn = getFieldDecorator('taskCandSltLimit', {
      rules: [
        {
          required: true,
          type: 'integer',
          message: I18N.get('from.TaskCreateForm.taskCandLimit.required')
        }
      ],
      initialValue: this.state.editing ? existingTask.candidateSltLimit : 1
    })
    const taskCandSltLimit_el = (
      <InputNumber size="large" disabled={hasLeaderEditRestrictions} />
    )

    const taskRewardUpfront_fn = getFieldDecorator('taskRewardUpfront', {
      initialValue:
        this.state.editing && existingTask.rewardUpfront.ela
          ? existingTask.rewardUpfront.ela / 1000
          : null
    })
    const taskRewardUpfront_el = (
      <InputNumber size="large" disabled={hasLeaderEditRestrictions} />
    )

    const taskRewardUpfrontUsd_fn = getFieldDecorator('taskRewardUpfrontUsd', {
      initialValue:
        this.state.editing && existingTask.rewardUpfront.usd
          ? existingTask.rewardUpfront.usd / 100
          : null
    })
    const taskRewardUpfrontUsd_el = (
      <InputNumber size="large" disabled={hasLeaderEditRestrictions} />
    )

    const taskRewardUsd_fn = getFieldDecorator('taskRewardUsd', {
      initialValue:
        this.state.editing && existingTask.reward.usd
          ? existingTask.reward.usd / 100
          : null
    })
    const taskRewardUsd_el = (
      <InputNumber size="large" disabled={hasLeaderEditRestrictions} />
    )

    const taskReward_fn = getFieldDecorator('taskReward', {
      initialValue:
        this.state.editing && existingTask.reward.ela
          ? existingTask.reward.ela / 1000
          : null
    })
    const taskReward_el = (
      <InputNumber size="large" disabled={hasLeaderEditRestrictions} />
    )

    const referenceBidOpts = {
      rules: [
        {
          validator: (rule, value, cb) => {
            if (
              value &&
              value !== '' &&
              (isNaN(parseFloat(value)) || parseFloat(value) <= 0)
            ) {
              cb('must be a number greater than 0')
              return
            }

            cb()
          }
        }
      ]
    }
    if (this.state.editing) {
      referenceBidOpts.initialValue = existingTask.referenceBid
    }

    const referenceBid_fn = getFieldDecorator('referenceBid', referenceBidOpts)
    const referenceBid_el = (
      <InputNumber size="large" disabled={hasLeaderEditRestrictions} />
    )

    const thumbnail_fn = getFieldDecorator('thumbnail', {
      rules: []
    })
    const p_thumbnail = {
      showUploadList: false,
      customRequest: info => {
        this.setState({
          thumbnail_loading: true
        })
        upload_file(info.file).then(d => {
          const url = d.url
          this.setState({
            thumbnail_loading: false,
            thumbnail_url: url,
            thumbnail_type: d.type,
            thumbnail_filename: d.filename,

            removeThumbnail: false
          })
        })
      }
    }
    const thumbnail_el = (
      <Upload name="logo" listType="picture" {...p_thumbnail}>
        {this.state.thumbnail_url ? (
          <img style={{ height: '100px' }} src={this.state.thumbnail_url} />
        ) : (
          <Button loading={this.state.thumbnail_loading}>
            <Icon type="upload" /> {I18N.get('from.TaskCreateForm.uploadtext')}
          </Button>
        )}
      </Upload>
    )

    const attachment_fn = getFieldDecorator('attachment', {
      rules: []
    })
    const p_attachment = {
      showUploadList: false,
      customRequest: info => {
        this.setState({
          attachment_loading: true
        })
        upload_file(info.file).then(d => {
          const url = d.url
          this.setState({
            attachment_loading: false,
            attachment_url: url,
            attachment_type: d.type,
            attachment_filename: d.filename,

            removeAttachment: false
          })
        })
      }
    }
    const attachment_el = (
      <Upload name="attachment" {...p_attachment}>
        {this.state.attachment_url ? (
          <a target="_blank" href={getSafeUrl(this.state.attachment_url)}>
            {this.state.attachment_type === 'application/pdf' ? (
              <Icon type="file-pdf" />
            ) : (
              <Icon type="file" />
            )}{' '}
            &nbsp;
            {this.state.attachment_filename}
          </a>
        ) : (
          <Button
            loading={this.state.attachment_loading}
            style={{ width: '100%' }}
          >
            <Icon type="upload" /> {I18N.get('from.TaskCreateForm.uploadtext')}
          </Button>
        )}
      </Upload>
    )

    const specs = [
      {
        title: I18N.get('team.spec.authenticity'),
        value: TEAM_TASK_DOMAIN.AUTHENTICITY,
        key: TEAM_TASK_DOMAIN.AUTHENTICITY
      },
      {
        title: I18N.get('team.spec.currency'),
        value: TEAM_TASK_DOMAIN.CURRENCY,
        key: TEAM_TASK_DOMAIN.CURRENCY
      },
      {
        title: I18N.get('team.spec.exchange'),
        value: TEAM_TASK_DOMAIN.EXCHANGE,
        key: TEAM_TASK_DOMAIN.EXCHANGE
      },
      {
        title: I18N.get('team.spec.finance'),
        value: TEAM_TASK_DOMAIN.FINANCE,
        key: TEAM_TASK_DOMAIN.FINANCE
      },
      {
        title: I18N.get('team.spec.gaming'),
        value: TEAM_TASK_DOMAIN.GAMING,
        key: TEAM_TASK_DOMAIN.GAMING
      },
      {
        title: I18N.get('team.spec.iot'),
        value: TEAM_TASK_DOMAIN.IOT,
        key: TEAM_TASK_DOMAIN.IOT
      },
      {
        title: I18N.get('team.spec.media'),
        value: TEAM_TASK_DOMAIN.MEDIA,
        key: TEAM_TASK_DOMAIN.MEDIA
      },
      {
        title: I18N.get('team.spec.social'),
        value: TEAM_TASK_DOMAIN.SOCIAL,
        key: TEAM_TASK_DOMAIN.SOCIAL
      },
      {
        title: I18N.get('team.spec.sovereignty'),
        value: TEAM_TASK_DOMAIN.SOVEREIGNTY,
        key: TEAM_TASK_DOMAIN.SOVEREIGNTY
      }
    ]

    const skillsets = [
      {
        title: I18N.get('team.skillset.cpp'),
        value: SKILLSET_TYPE.CPP,
        key: SKILLSET_TYPE.CPP
      },
      {
        title: I18N.get('team.skillset.javascript'),
        value: SKILLSET_TYPE.JAVASCRIPT,
        key: SKILLSET_TYPE.JAVASCRIPT
      },
      {
        title: I18N.get('team.skillset.go'),
        value: SKILLSET_TYPE.GO,
        key: SKILLSET_TYPE.GO
      },
      {
        title: I18N.get('team.skillset.python'),
        value: SKILLSET_TYPE.PYTHON,
        key: SKILLSET_TYPE.PYTHON
      },
      {
        title: I18N.get('team.skillset.java'),
        value: SKILLSET_TYPE.JAVA,
        key: SKILLSET_TYPE.JAVA
      },
      {
        title: I18N.get('team.skillset.swift'),
        value: SKILLSET_TYPE.SWIFT,
        key: SKILLSET_TYPE.SWIFT
      }
    ]

    const domain_fn = getFieldDecorator('domain', {
      rules: [],
      initialValue:
        (this.props.existingTask && this.props.existingTask.domain) || []
    })
    const domain_el = (
      <TreeSelect
        treeData={specs}
        treeCheckable={true}
        searchPlaceholder={I18N.get('select.placeholder')}
      />
    )

    const skillset_fn = getFieldDecorator('recruitedSkillsets', {
      rules: [],
      initialValue:
        (this.props.existingTask &&
          this.props.existingTask.recruitedSkillsets) ||
        []
    })
    const skillset_el = (
      <TreeSelect
        treeData={skillsets}
        treeCheckable={true}
        searchPlaceholder={I18N.get('select.placeholder')}
      />
    )

    const p_pictures = {
      listType: 'picture-card',
      fileList: this.state.fileList,
      onChange: this.handleFileListChange.bind(this),
      onPreview: this.handlePreview.bind(this),
      customRequest: info => {
        upload_file(info.file).then(d => {
          this.pictureUrlLookups = this.pictureUrlLookups || []
          this.pictureUrlLookups[info.file.uid] = d.url
          info.onSuccess(null, info.file)
        }, info.onError)
      }
    }

    const uploadButton = (
      <div>
        <Icon type="plus" />
        <div className="ant-upload-text">Upload</div>
      </div>
    )

    const pictures_el = (
      <Upload name="pictures" {...p_pictures}>
        {this.state.fileList.length >= 5 ? null : uploadButton}
      </Upload>
    )

    const problem_fn = getFieldDecorator('problem', {
      rules: [
        { max: 4096, message: I18N.get('from.TaskCreateForm.problem.max') }
      ],
      initialValue: this.state.editing
        ? existingTask.pitch && existingTask.pitch.problem
        : ''
    })
    const problem_el = <TextArea rows={4} />

    const valueProposition_fn = getFieldDecorator('valueProposition', {
      rules: [
        { max: 4096, message: I18N.get('from.TaskCreateForm.problem.max') }
      ],
      initialValue: this.state.editing
        ? existingTask.pitch && existingTask.pitch.valueProposition
        : ''
    })
    const valueProposition_el = <TextArea rows={4} />

    const usecase_fn = getFieldDecorator('useCase', {
      rules: [
        { max: 4096, message: I18N.get('from.TaskCreateForm.problem.max') }
      ],
      initialValue: this.state.editing
        ? existingTask.pitch && existingTask.pitch.useCase
        : ''
    })
    const usecase_el = <TextArea rows={4} />

    const beneficiaries_fn = getFieldDecorator('beneficiaries', {
      rules: [
        { max: 4096, message: I18N.get('from.TaskCreateForm.problem.max') }
      ],
      initialValue: this.state.editing
        ? existingTask.pitch && existingTask.pitch.beneficiaries
        : ''
    })
    const beneficiaries_el = <TextArea rows={4} />

    const elaInfrastructure_fn = getFieldDecorator('elaInfrastructure', {
      rules: [
        { max: 4096, message: I18N.get('from.TaskCreateForm.problem.max') }
      ],
      initialValue: this.state.editing
        ? existingTask.pitch && existingTask.pitch.elaInfrastructure
        : ''
    })
    const elaInfrastructure_el = <TextArea rows={4} />

    return {
      problem: problem_fn(problem_el),
      valueProposition: valueProposition_fn(valueProposition_el),
      usecase: usecase_fn(usecase_el),
      beneficiaries: beneficiaries_fn(beneficiaries_el),
      elaInfrastructure: elaInfrastructure_fn(elaInfrastructure_el),

      recruitedSkillsets: skillset_fn(skillset_el),
      pictures: pictures_el,
      domain: domain_fn(domain_el),

      taskName: taskName_fn(taskName_el),
      taskCategory: taskCategory_fn(taskCategory_el),
      taskType: taskType_fn(taskType_el),

      taskApplicationDeadline: applicationDeadline_fn(applicationDeadline_el),
      taskCompletionDeadline: completionDeadline_fn(completionDeadline_el),

      taskCommunity: taskCommunity_fn(taskCommunity_el),

      eventDateStatus: eventDateStatus_fn(eventDateStatus_el),
      eventDateRange: eventDateRange_fn(eventDateRange_el),
      eventDateRangeStart: eventDateRangeStart_fn(eventDateRangeStart_el),
      eventDateRangeEnd: eventDateRangeEnd_fn(eventDateRangeEnd_el),

      taskLocation: taskLocation_fn(taskLocation_el),

      taskDesc: taskDesc_fn(taskDesc_el),
      taskDescBreakdown: taskDescBreakdown_fn(taskDescBreakdown_el),
      taskGoals: taskGoals_fn(taskGoals_el),
      taskLink: taskLink_fn(taskLink_el),
      taskCandLimit: taskCandLimit_fn(taskCandLimit_el),
      taskCandSltLimit: taskCandSltLimit_fn(taskCandSltLimit_el),

      taskRewardUpfront: taskRewardUpfront_fn(taskRewardUpfront_el),
      taskRewardUpfrontUsd: taskRewardUpfrontUsd_fn(taskRewardUpfrontUsd_el),

      taskReward: taskReward_fn(taskReward_el),
      taskRewardUsd: taskRewardUsd_fn(taskRewardUsd_el),

      referenceBid: referenceBid_fn(referenceBid_el),

      thumbnail: thumbnail_fn(thumbnail_el),

      // TODO: fix issue where existing attachment can't be removed
      attachment: attachment_fn(attachment_el),
      circle: circle_fn(circle_el)
    }
  }

  async getCommunityTrees() {
    const communityTrees = await this.props.getAllCommunities()

    await this.setState({
      communityTrees
    })
  }

  handleCancel() {
    this.setState({ previewVisible: false })
  }

  handlePreview(file) {
    this.setState({
      previewImage: file.url || file.thumbUrl,
      previewVisible: true
    })
  }

  handleFileListChange = ({ fileList }) => this.setState({ fileList })

  ord_render() {
    const { getFieldDecorator } = this.props.form
    const p = this.getInputProps()
    const existingTask = this.props.existingTask

    const formItemLayout = {
      labelCol: {
        xs: { span: 24 },
        sm: { span: 8 }
      },
      wrapperCol: {
        xs: { span: 24 },
        sm: { span: 12 }
      }
    }

    const formItemLayoutAdjLeft = {
      labelCol: {
        xs: { span: 24 },
        sm: { span: 16 }
      },
      wrapperCol: {
        xs: { span: 24 },
        sm: { span: 8 }
      }
    }

    const formItemLayoutAdjRight = {
      labelCol: {
        xs: { span: 24 },
        sm: { span: 8 }
      },
      wrapperCol: {
        xs: { span: 24 },
        sm: { span: 16 }
      }
    }

    const formItemNoLabelLayout = {
      wrapperCol: {
        xs: { span: 24 },
        sm: { offset: 8, span: 12 }
      }
    }

    const formItemCenterLayout = {
      wrapperCol: {
        xs: { span: 24 },
        sm: { offset: 6, span: 12 }
      }
    }

    // const existingTask = this.props.existingTask

    // TODO: terms of service checkbox\

    // TODO: react-motion animate slide left

    // TODO: description CKE Editor
    return (
      <div className="c_taskCreateFormContainer">
        <Form
          onSubmit={this.handleSubmit.bind(this)}
          className="d_taskCreateForm"
        >
          <div>
            <h3 className="no-margin">General Info</h3>
            <FormItem
              label={I18N.get('from.TaskCreateForm.label.name')}
              {...formItemLayout}
            >
              {p.taskName}
            </FormItem>
            {this.props.taskType !== 'PROJECT' && (
              <FormItem
                label={I18N.get('from.TaskCreateForm.label.assigntocircle')}
                {...formItemLayout}
              >
                {p.circle}
              </FormItem>
            )}
            <FormItem
              label={I18N.get('from.TaskCreateForm.label.community')}
              {...formItemLayout}
            >
              {p.taskCommunity}
            </FormItem>

            {!this.state.thumbnail_url ? (
              <FormItem
                label={I18N.get('from.TaskCreateForm.label.thumbnail')}
                {...formItemLayout}
              >
                {p.thumbnail}
              </FormItem>
            ) : (
              <Row className="ant-form-item">
                <Col span={8} className="ant-form-item-label text-right">
                  <label>
                    {I18N.get('from.TaskCreateForm.label.thumbnail')}
                  </label>
                </Col>
                <Col span={16} style={{ lineHeight: '40px' }}>
                  <a target="_blank" href={getSafeUrl(this.state.thumbnail)}>
                    <Icon type="file-image" />
                    {this.state.thumbnail_filename}
                  </a>
                  <Popconfirm
                    title={I18N.get(
                      'from.TaskCreateForm.attachment.confirm.remove'
                    )}
                    okText={I18N.get('from.TaskCreateForm.text.ok')}
                    onConfirm={this.removeThumbnail.bind(this)}
                  >
                    <Icon className="remove-attachment" type="close-circle" />
                  </Popconfirm>
                  <br />
                </Col>
              </Row>
            )}
            <FormItem
              label={I18N.get('from.TaskCreateForm.label.category')}
              {...formItemLayout}
            >
              {p.taskCategory}
            </FormItem>
            {this.props.taskType !== 'PROJECT' && (
              <FormItem
                label={I18N.get('from.TaskCreateForm.label.type')}
                {...formItemLayout}
              >
                {p.taskType}
              </FormItem>
            )}
            <Row>
              <Col span={12}>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.application')}
                  {...formItemLayoutAdjLeft}
                >
                  {p.taskApplicationDeadline}
                </FormItem>
              </Col>
              <Col span={12}>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.completeBy')}
                  {...formItemLayoutAdjRight}
                >
                  {p.taskCompletionDeadline}
                </FormItem>
              </Col>
            </Row>
            <FormItem
              label={I18N.get('from.TaskCreateForm.label.description')}
              {...formItemLayout}
            >
              {p.taskDesc}
            </FormItem>

            <Row>
              <Col offset={8} span={12}>
                For larger events/tasks/projects please breakdown the
                budget/rewards
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.taskDescBreakdown}
            </FormItem>
            <FormItem
              label={I18N.get('from.TaskCreateForm.label.goals')}
              {...formItemLayout}
            >
              {p.taskGoals}
            </FormItem>
            <FormItem
              label={I18N.get('from.TaskCreateForm.label.info')}
              {...formItemLayout}
            >
              {p.taskLink}
            </FormItem>

            {((existingTask && existingTask.type === TASK_TYPE.PROJECT) ||
              this.state.taskType === TASK_TYPE.PROJECT) && (
              <div>
                <h3 className="no-margin">Pitch</h3>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.problems')}
                  {...formItemLayout}
                >
                  {p.problem}
                </FormItem>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.valueProposition')}
                  {...formItemLayout}
                >
                  {p.valueProposition}
                </FormItem>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.usecase')}
                  {...formItemLayout}
                >
                  {p.usecase}
                </FormItem>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.beneficiaries')}
                  {...formItemLayout}
                >
                  {p.beneficiaries}
                </FormItem>
                <FormItem
                  label={I18N.get(
                    'from.TaskCreateForm.label.elaInfrastructure'
                  )}
                  {...formItemLayout}
                >
                  {p.elaInfrastructure}
                </FormItem>

                <h3 className="no-margin">Recruitment</h3>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.domain')}
                  {...formItemLayout}
                >
                  {p.domain}
                </FormItem>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.recruiting')}
                  {...formItemLayout}
                >
                  {p.recruitedSkillsets}
                </FormItem>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.prictures')}
                  {...formItemLayout}
                >
                  {p.pictures}
                </FormItem>
                <Modal
                  visible={this.state.previewVisible}
                  footer={null}
                  onCancel={this.handleCancel.bind(this)}
                >
                  <img
                    alt="example"
                    style={{ width: '100%' }}
                    src={this.state.previewImage}
                  />
                </Modal>
              </div>
            )}

            <Divider />

            {/*
             ********************************************************************************
             * Event Info
             ********************************************************************************
             */}
            {this.state.taskType === TASK_TYPE.EVENT && (
              <div>
                <h3 className="no-margin">Event Info</h3>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.daterange')}
                  {...formItemLayout}
                >
                  {p.eventDateRange}
                </FormItem>
                <Row>
                  <Col span={this.state.eventDateRange ? 12 : 24}>
                    {this.state.eventDateRange ? (
                      <FormItem
                        label={
                          I18N.get('from.TaskCreateForm.label.eventdate') +
                          (this.state.eventDateRange
                            ? I18N.get('from.TaskCreateForm.label.start')
                            : '')
                        }
                        {...formItemLayoutAdjLeft}
                      >
                        {p.eventDateRangeStart}
                      </FormItem>
                    ) : (
                      <FormItem
                        label={
                          I18N.get('from.TaskCreateForm.label.eventdate') +
                          (this.state.eventDateRange
                            ? I18N.get('from.TaskCreateForm.label.start')
                            : '')
                        }
                        {...formItemLayout}
                      >
                        {p.eventDateRangeStart}
                      </FormItem>
                    )}
                  </Col>
                  {this.state.eventDateRange && (
                    <Col span={12}>
                      <FormItem
                        label={I18N.get('from.TaskCreateForm.label.end')}
                        {...formItemLayoutAdjRight}
                      >
                        {p.eventDateRangeEnd}
                      </FormItem>
                    </Col>
                  )}
                </Row>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.dateconfirm')}
                  {...formItemLayout}
                >
                  {p.eventDateStatus}
                </FormItem>
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.location')}
                  {...formItemLayout}
                >
                  {p.taskLocation}
                </FormItem>
              </div>
            )}

            {this.state.taskType === TASK_TYPE.EVENT && <Divider />}

            {/*
             ********************************************************************************
             * Budget / Reward
             ********************************************************************************
             */}
            <h3 className="no-margin">
              {I18N.get('from.TaskCreateForm.label.paymentassigment')}
              &nbsp;
              <Popover
                content={I18N.get('from.TaskCreateForm.label.budgetlabor')}
              >
                <Icon className="help-icon" type="question-circle-o" />
              </Popover>
            </h3>
            {!this.props.existingTask && (
              <Row>
                <Col span={12}>
                  <Card
                    hoverable={true}
                    className={`feature-box${
                      this.state.assignSelf ? ' selected' : ''
                    }`}
                    onClick={() => {
                      this.setState({ assignSelf: true })
                    }}
                  >
                    <div className="title">
                      <span>
                        {I18N.get('from.TaskCreateForm.label.private')}
                      </span>
                    </div>
                    <hr className="feature-box-divider" />
                    <div className="content">
                      <div>
                        {I18N.get('from.TaskCreateForm.label.taskyourself')}
                      </div>
                      <div>
                        {I18N.get(
                          'from.TaskCreateForm.label.proposing.approval'
                        )}
                      </div>
                      <div>
                        {I18N.get('from.TaskCreateForm.label.notvisible')}
                      </div>
                    </div>
                  </Card>
                </Col>
                <Col span={12}>
                  <Card
                    hoverable={true}
                    className={`feature-box${
                      !this.state.assignSelf ? ' selected' : ''
                    }`}
                    onClick={() => {
                      this.setState({ assignSelf: false })
                    }}
                  >
                    <div className="title">
                      <span>
                        {I18N.get('from.TaskCreateForm.label.public')}
                      </span>
                    </div>
                    <hr className="feature-box-divider" />
                    <div className="content">
                      <div>
                        {I18N.get('from.TaskCreateForm.label.taskforother')}
                      </div>
                      <div>
                        {I18N.get('from.TaskCreateForm.label.publicly')}
                      </div>
                      <div>
                        {I18N.get('from.TaskCreateForm.label.rewardbidding')}
                      </div>
                    </div>
                  </Card>
                </Col>
              </Row>
            )}

            {!this.state.assignSelf && (
              <div>
                <br />
                <FormItem
                  label={I18N.get('from.TaskCreateForm.label.rewardtype')}
                  {...formItemLayout}
                >
                  <Switch
                    onChange={() =>
                      this.setState({ isBidding: !this.state.isBidding })
                    }
                    unCheckedChildren="Define budget"
                    checkedChildren="Open for bidding"
                    defaultChecked={this.state.isBidding}
                  />
                </FormItem>
              </div>
            )}
            {(this.state.assignSelf || !this.state.isBidding) && (
              <div>
                {this.state.assignSelf && <br />}

                <Row>
                  <Col>
                    <FormItem
                      label={I18N.get('from.TaskCreateForm.label.usdbudget')}
                      {...formItemLayout}
                    >
                      {p.taskRewardUpfrontUsd}
                    </FormItem>
                    <FormItem
                      label={I18N.get('from.TaskCreateForm.label.usdreward')}
                      {...formItemLayout}
                    >
                      {p.taskRewardUsd}
                    </FormItem>
                  </Col>
                </Row>

                {!this.props.is_admin &&
                  (!this.props.existingTask ||
                    this.props.existingTask.status === TASK_STATUS.PENDING) && (
                    <FormItem {...formItemNoLabelLayout}>
                      <Checkbox
                        name="readDisclaimer"
                        checked={this.state.readDisclaimer}
                        onChange={() => {
                          this.setState({
                            readDisclaimer: !this.state.readDisclaimer
                          })
                        }}
                      />

                      <span
                        id="disclaimerLink"
                        className="disclaimerLink"
                        onClick={this.showDisclaimer.bind(this)}
                      >
                        {I18N.get('from.TaskCreateForm.label.disclaimerrule')}
                      </span>
                    </FormItem>
                  )}
              </div>
            )}

            {!this.state.assignSelf && this.state.isBidding && (
              <FormItem
                label={I18N.get('project.detail.reference_bid')}
                {...formItemLayout}
              >
                {p.referenceBid}
              </FormItem>
            )}

            <Divider />

            {/*
             ********************************************************************************
             * Attachment
             ********************************************************************************
             */}
            <h3 className="no-margin">
              {I18N.get('from.TaskCreateForm.label.attachment')}
            </h3>
            <br />
            {!this.state.attachment_url ? (
              <FormItem {...formItemCenterLayout} className="attachmentUpload">
                {p.attachment}
              </FormItem>
            ) : (
              <Row>
                <Col offset={8} span={16}>
                  <a
                    target="_blank"
                    href={getSafeUrl(this.state.attachment_url)}
                  >
                    {this.state.attachment_type === 'application/pdf' ? (
                      <Icon type="file-pdf" />
                    ) : (
                      <Icon type="file" />
                    )}{' '}
                    &nbsp;
                    {this.state.attachment_filename}
                  </a>
                  <Popconfirm
                    title={I18N.get(
                      'from.TaskCreateForm.label.remove.attachment'
                    )}
                    okText="Yes"
                    onConfirm={this.removeAttachment.bind(this)}
                  >
                    <Icon className="remove-attachment" type="close-circle" />
                  </Popconfirm>
                  <br />
                </Col>
              </Row>
            )}

            <Divider />

            <br />
            <Row style={{ margin: '50px 0 100px 0' }}>
              <Col offset={4} span={16}>
                <Button
                  loading={this.props.loading || this.props.all_circles_loading}
                  type="primary"
                  htmlType="submit"
                  className="d_btn"
                  style={{ width: '100%' }}
                >
                  {this.state.editing
                    ? I18N.get('from.TaskCreateForm.button.savechange')
                    : this.props.is_admin
                    ? I18N.get('from.TaskCreateForm.button.createtask')
                    : I18N.get('from.TaskCreateForm.button.submitpropsal')}
                </Button>
              </Col>
            </Row>

            <br />
          </div>
        </Form>

        <Modal
          title={I18N.get('from.TaskCreateForm.label.paymentrules')}
          visible={this.state.showDisclaimer}
          onCancel={this.hideDisclaimer.bind(this)}
          footer={[
            <Button key="cancel" onClick={this.hideDisclaimer.bind(this)}>
              {I18N.get('from.TaskCreateForm.button.close')}
            </Button>
          ]}
        >
          <ol className="paymentRulesModal">
            <li>{I18N.get('from.TaskCreateForm.text.payment.billable')}</li>
            <li>{I18N.get('from.TaskCreateForm.text.payment.upon')}</li>
            <li>
              {I18N.get('from.TaskCreateForm.text.payment.exchange')}{' '}
              <a
                target="_blank"
                href="https://coinmarketcap.com/currencies/elastos"
              >
                CMC
              </a>{' '}
              {I18N.get('from.TaskCreateForm.text.payment.used')}
            </li>
            <li>{I18N.get('from.TaskCreateForm.text.payment.expenses')}</li>
            <li>{I18N.get('from.TaskCreateForm.text.payment.agreement')}</li>
          </ol>
        </Modal>
      </div>
    )
  }

  removeAttachment() {
    this.setState({
      attachment_loading: false,
      attachment_url: null,
      attachment_type: '',
      attachment_filename: '',

      removeAttachment: true
    })
  }

  removeThumbnail() {
    this.setState({
      thumbnail_loading: false,
      thumbnail_url: null,
      thumbnail_type: '',
      thumbnail_filename: '',

      removeThumbnail: true
    })
  }

  showDisclaimer() {
    this.setState({
      showDisclaimer: true
    })
  }

  hideDisclaimer() {
    this.setState({
      showDisclaimer: false
    })
  }
}
export default Form.create()(C)
