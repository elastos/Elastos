import { TASK_STATUS, TASK_CANDIDATE_STATUS } from '@/constant'
import _ from 'lodash'
import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import TaskCreateForm from '@/module/form/TaskCreateForm/Container'
import { Icon, Popconfirm, Menu, Button, Spin, Dropdown } from 'antd'
import I18N from '@/I18N'

// TODO: admin detail should also be in a new component too to be consistent
import ProjectPublicDetail from '@/module/project/detail/Container'

import './style.scss'
import moment from 'moment/moment'

/**
 * This has 3 views
 *
 * 1. Public
 * 2. Admin
 * 3. Edit
 *
 */
export default class extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      editing: false
    }
  }

  async componentDidMount() {
    const taskId = this.props.match.params.taskId
    await this.props.getTaskDetail(taskId)
  }

  renderMain() {
    return (
      <div className="c_TaskDetail">
        {this.props.page === 'ADMIN' || this.props.is_admin
          ? this.renderAdminHeader()
          : this.renderHeader()}
        {this.state.editing ? this.renderEditForm() : this.renderDetail()}
      </div>
    )
  }

  renderEditForm() {
    return (
      <div className="form-wrapper">
        <TaskCreateForm
          existingTask={this.props.task}
          page={this.props.page}
          switchEditMode={this.switchEditMode.bind(this)}
        />
      </div>
    )
  }

  renderDetail() {
    return <ProjectPublicDetail taskId={this.props.task._id} />
  }

  // not using I18N here since all admins so far know English and this isn't visible to non-admins
  renderAdminHeader() {
    const adminDropdown = this.buildAdminDropdown()
    const curTask = this.props.task

    return (
      <div className="l_banner">
        {curTask.category !== 'CR100' ? (
          <div className="pull-left">
            {I18N.get('admin.tasks.status')}:
            <span className="status">
              {I18N.get(`taskStatus.${curTask.status}`)}
            </span>
            {curTask.status === TASK_STATUS.CREATED && (
              <span className="help-text">
                &nbsp; -{I18N.get('project.admin.statusHelp.created')}
              </span>
            )}
            {curTask.status === TASK_STATUS.PENDING && (
              <span className="help-text">
                &nbsp; -{I18N.get('project.admin.statusHelp.pending')}
              </span>
            )}
            {(curTask.status === TASK_STATUS.APPROVED ||
              curTask.status === TASK_STATUS.ASSIGNED) &&
              curTask.approvedBy && (
                <span className="help-text">
                  &nbsp; - {I18N.get('project.detail.statusHelp.approvedBy')}{' '}
                  {curTask.approvedBy.username}
                  {curTask.approvedDate &&
                    ` ${I18N.get(
                      'project.admin.statusHelp.approvedOn'
                    )} ${moment(curTask.approvedDate).format('MMM D')}`}
                </span>
              )}
            {curTask.status === TASK_STATUS.SUCCESS &&
              (curTask.reward.ela > 0 || curTask.reward.usd > 0 ? (
                <span className="help-text">
                  &nbsp; -{I18N.get('project.admin.statusHelp.successReward')}
                </span>
              ) : (
                <span className="help-text">
                  &nbsp; -{I18N.get('project.admin.statusHelp.successNoReward')}
                </span>
              ))}
          </div>
        ) : (
          <div className="pull-left">
            CR100 {I18N.get('developer.search.project')}
          </div>
        )}
        {curTask.category !== 'CR100' && (
          <div className="pull-right right-align">
            {!this.state.editing && curTask.status === TASK_STATUS.PENDING && (
              <Popconfirm
                title="Are you sure you want to approve this task?"
                placement="left"
                okText="Yes"
                onConfirm={this.approveTask.bind(this)}
              >
                <Button>Approve</Button>
              </Popconfirm>
            )}
            {!this.state.editing &&
              curTask.status === TASK_STATUS.PENDING &&
              curTask.assignSelf !== true && (
                <Popconfirm
                  title="Are you sure you want to approve this task and assign to the owner?"
                  placement="left"
                  okText="Yes"
                  onConfirm={this.approveAndAssignTask.bind(this)}
                >
                  <Button>Approve and Assign to Owner</Button>
                </Popconfirm>
              )}
            {/* ONLY Admin & Task Owner CAN Mark as Complete */}
            {(curTask.status === TASK_STATUS.APPROVED ||
              curTask.status === TASK_STATUS.ASSIGNED) && (
              <Popconfirm
                title="Are you sure you want to mark this task as complete?"
                placement="left"
                okText="Yes"
                onConfirm={this.markAsSubmitted.bind(this)}
              >
                <Button>Mark as Complete</Button>
              </Popconfirm>
            )}
            {!this.state.editing && curTask.status === TASK_STATUS.SUBMITTED && (
              <Popconfirm
                title="Are you sure you want to accept this task as completed?"
                placement="left"
                okText="Yes"
                onConfirm={this.markAsSuccessful.bind(this)}
              >
                <Button>Accept as Complete</Button>
              </Popconfirm>
            )}
            {!this.state.editing &&
              curTask.status === TASK_STATUS.SUCCESS &&
              (curTask.reward.ela > 0 || curTask.reward.usd > 0) && (
                <Popconfirm
                  title="Are you sure you want to mark the ELA as disbursed?"
                  placement="left"
                  okText="Yes"
                  onConfirm={this.markAsDisbursed.bind(this)}
                >
                  <Button type="primary">Mark as Disbursed</Button>
                </Popconfirm>
              )}
            {/* this.state.editing && <Button onClick={this.resetEdit.bind(this)}>Reset</Button> */}
            <Button onClick={this.switchEditMode.bind(this)}>
              {this.state.editing ? 'Cancel' : 'Edit'}
            </Button>
            <Dropdown overlay={adminDropdown}>
              <a className="ant-dropdown-link">
                <Icon
                  type="bars"
                  theme="outlined"
                  style={{
                    color: 'white',
                    fontSize: '24px',
                    verticalAlign: 'middle'
                  }}
                />
              </a>
            </Dropdown>
          </div>
        )}
        <div className="clearfix" />
      </div>
    )
  }

  renderHeader() {
    const isTaskOwner =
      this.props.current_user_id ===
      (this.props.task.createdBy && this.props.task.createdBy._id)

    const taskAssignee = _.filter(this.props.task.candidates, {
      status: TASK_CANDIDATE_STATUS.APPROVED
    })
    let isTaskAssignee = false

    if (taskAssignee.length) {
      isTaskAssignee = taskAssignee[0].user._id === this.props.current_user_id
    }

    return (
      <div className="l_banner">
        {this.props.task.category !== 'CR100' ? (
          <div className="pull-left">
            {I18N.get('admin.tasks.status')}:
            <span className="status">
              {I18N.get(`taskStatus.${this.props.task.status}`)}
            </span>
            {this.props.task.status === TASK_STATUS.PENDING && (
              <span className="help-text">
                &nbsp; -{I18N.get('project.public.statusHelp.pending')}
              </span>
            )}
            {this.props.task.status === TASK_STATUS.SUBMITTED && (
              <span className="help-text">
                &nbsp; -{I18N.get('project.public.statusHelp.submitted')}
              </span>
            )}
            {this.props.task.status === TASK_STATUS.SUCCESS && (
              <span className="help-text">
                &nbsp; -{I18N.get('project.public.statusHelp.success')}
              </span>
            )}
          </div>
        ) : (
          <div className="pull-left">CR100 Project</div>
        )}
        {this.props.task.category !== 'CR100' && (
          <div className="pull-right right-align">
            {/* Admin & Task Assignee CAN Mark as Complete - TODO: we should consider if we need a separate flag to allow the owner to approve the deliverable */}
            {(this.props.task.status === TASK_STATUS.APPROVED ||
              this.props.task.status === TASK_STATUS.ASSIGNED) &&
              isTaskAssignee && (
                <Popconfirm
                  title={I18N.get(
                    'project.public.statusHelp.markAsCompleteConfirm'
                  )}
                  placement="left"
                  okText={I18N.get('.yes')}
                  onConfirm={this.markAsSubmitted.bind(this)}
                >
                  <Button>
                    {I18N.get('project.public.statusHelp.markAsComplete')}
                  </Button>
                </Popconfirm>
              )}
            {isTaskOwner &&
              _.includes(
                [TASK_STATUS.CREATED, TASK_STATUS.PENDING],
                this.props.task.status
              ) && (
                <Button onClick={this.switchEditMode.bind(this)}>
                  {this.state.editing ? I18N.get('.cancel') : I18N.get('.edit')}
                </Button>
              )}
          </div>
        )}
        <div className="clearfix" />
      </div>
    )
  }

  ord_render() {
    return _.isEmpty(this.props.task) || this.props.task.loading ? (
      <div className="center">
        <Spin size="large" />
      </div>
    ) : (
      this.renderMain()
    )
  }

  // TODO: DRY - move to helper
  getCommunityDisp() {
    let str = ''
    if (this.props.task.communityParent) {
      str += `${this.props.task.communityParent.name}/`
    }
    if (this.props.task.community) {
      str += this.props.task.community.name
    }

    return str
  }

  async approveTask() {
    const taskId = this.props.task._id
    await this.props.approveTask(taskId)
  }

  async approveAndAssignTask() {
    const taskId = this.props.task._id
    await this.props.approveAndAssignTask(taskId)
  }

  async markAsSuccessful() {
    const taskId = this.props.task._id
    await this.props.markAsSuccessful(taskId)
  }

  async markAsSubmitted() {
    const taskId = this.props.task._id
    await this.props.markAsSubmitted(taskId)
  }

  async markAsDisbursed() {
    const taskId = this.props.task._id
    await this.props.markAsDisbursed(taskId)
  }

  async forceStart() {
    const taskId = this.props.task._id
    await this.props.forceStart(taskId)
  }

  async saveTask() {}

  async clickAdminAction(o) {
    const taskId = this.props.task._id

    switch (o.key) {
      case 'archive':
        await this.props.archive(taskId)
        break
    }
  }

  buildAdminDropdown() {
    return (
      <Menu onClick={this.clickAdminAction.bind(this)} className="help-menu">
        <Menu.Item key="text" disabled={true}>
          Admin Only
        </Menu.Item>
        <Menu.Divider />
        <Menu.Item key="markBudgetPaid">Mark upfront budget as paid</Menu.Item>
        <Menu.Item key="archive">Archive Task</Menu.Item>
      </Menu>
    )
  }

  async switchEditMode() {
    if (!this.state.editing) {
      this.setState({ editing: true })
    } else {
      window.location.reload()
    }
    // temp hack above till we figure out why editing lifecycle has spinner
    // might be the community loader call
    // this.setState({editing: !this.state.editing})
  }
}
