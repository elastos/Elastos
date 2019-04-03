import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { TASK_CANDIDATE_STATUS, TEAM_USER_STATUS, USER_AVATAR_DEFAULT, TASK_AVATAR_DEFAULT } from '@/constant'
import {Avatar, Button, Col, Form, Icon, Popconfirm, Row, Spin, Table, Input, Modal} from 'antd'
import Comments from '@/module/common/comments/Container'
import LoginOrRegisterForm from '@/module/form/LoginOrRegisterForm/Container'
import TaskCreateForm from '@/module/form/TaskCreateForm/Container'
import TaskDetail from '@/module/task/popup/Container'
import ProfilePopup from '@/module/profile/OverviewPopup/Container'
import './style.scss'
import _ from 'lodash'
import I18N from '@/I18N'

const FormItem = Form.Item
const { TextArea } = Input

class C extends BaseComponent {
  ord_states() {
    return {
      showLoginRegisterModal: false,
      showTaskModal: false,
      showUserInfo: null,
      taskDetailId: null,
      showCreateModal: null
    }
  }

  componentDidMount() {
    const teamId = this.props.match.params.circleId
    this.props.getTeamDetail(teamId)
    this.props.getCircleTasks(teamId)
  }

  componentWillUnmount() {
    this.props.resetTeamDetail()
  }

  renderMain() {
    return (
      <div className="c_CircleDetail">
        {this.renderDetail()}
      </div>
    )
  }

  isTeamMember() {
    return _.find(this.props.team.detail.members, (member) => {
      return member.user._id === this.props.currentUserId &&
                member.status === TEAM_USER_STATUS.NORMAL
    })
  }

  hasApplied() {
    return _.find(this.props.team.detail.members, (member) => {
      return member.user._id === this.props.currentUserId &&
                member.status === TEAM_USER_STATUS.PENDING
    })
  }

  async leaveTeam() {
    const member = _.find(this.props.team.detail.members, (member) => {
      return member.user._id === this.props.currentUserId &&
                member.status === TEAM_USER_STATUS.NORMAL
    })

    if (member) {
      this.props.withdrawCandidate(member._id)
    }
  }

  applyToCircle() {
    if (this.props.is_login) {
      this.props.applyToTeam(this.props.match.params.circleId,
        this.props.currentUserId)
    } else {
      this.showLoginRegisterModal()
    }
  }

  createNewTask() {
    if (this.props.is_login) {
      this.showCreateTaskModal()
    } else {
      this.showLoginRegisterModal()
    }
  }

  viewTask(taskId) {

  }

  getMainActions() {
    const isTeamMember = this.isTeamMember()
    const hasApplied = this.hasApplied()
    const maxReached = _.size(this.props.myCircles) >= 2
    const mainActionButton = isTeamMember
      ? (
        <Popconfirm title={I18N.get('project.detail.popup.leave_question')}
          okText={I18N.get('.yes')} cancelText={I18N.get('.no')}
          onConfirm={this.leaveTeam.bind(this)}>
          <Button disabled={true} className="cr-btn" type="primary" loading={this.props.loading}>
            {I18N.get('circle.header.leave')}
          </Button>
        </Popconfirm>
      )
      : (
        <Button disabled={true} className="join-button cr-btn" onClick={() => this.applyToCircle()}
          loading={this.props.loading}>
          { hasApplied
            ? I18N.get('project.detail.popup.applied')
            : maxReached
              ? I18N.get('circle.header.maxReached')
              : I18N.get('circle.header.join')
          }
        </Button>
      )

    return (
      <Row className="actions">
        {mainActionButton}
      </Row>
    )
  }

  renderHeader() {
    return (
      <div className="header-container">
        <img className="circle-rectangle" src="/assets/images/emp35/circle_rectangle.png"/>
        <div className="circle-name komu-a">
          {
            this.props.loading
              ? <br/>
              : this.props.team.detail.name || <br/>
          }
        </div>
        <Row>
          <Col span={8} className="left-col">
            <div className="circle-contributor-number komu-a">{_.size(this.props.team.detail.comments)}</div>
            <span className="circle-contributor-label synthese">{I18N.get('circle.posts')}</span>
          </Col>
          <Col span={8}>
            {this.getMainActions()}
            <img className="circle-down-arrow" src="/assets/images/emp35/down_arrow.png"/>
          </Col>
          <Col span={8} className="right-col">
            <div className="circle-members-number komu-a">{_.size(this.props.team.detail.members)}</div>
            <span className="circle-members-label synthese">{I18N.get('circle.members')}</span>
          </Col>
        </Row>
      </div>
    )
  }

  linkProfileInfo(user) {
    this.setState({
      showUserInfo: user
    })
    // this.props.history.push(`/member/${userId}`)
  }

  renderContent() {
    return (
      <div className="content-paragraphs">
        <p className="synthese">{I18N.get('emp35.circles.statement')}</p>
      </div>
    )
  }

  getAvatarWithFallback(avatar) {
    return _.isEmpty(avatar)
      ? USER_AVATAR_DEFAULT
      : avatar
  }

  getTaskAvatarWithFallback(avatar) {
    return _.isEmpty(avatar)
      ? TASK_AVATAR_DEFAULT
      : avatar
  }

  getUserNameWithFallback(user) {
    if (_.isEmpty(user.profile.firstName) && _.isEmpty(user.profile.lastName)) {
      return user.username
    }

    return _.trim([user.profile.firstName, user.profile.lastName].join(' '))
  }

  renderMembers() {
    const members = _.filter(this.props.team.detail.members, { status: TEAM_USER_STATUS.NORMAL })
    const columns = [{
      title: 'Member',
      key: 'name',
      render: candidate => {
        return (
          <div key={candidate._id}>
            <Avatar className={`gap-right ${candidate.role === 'LEADER' ? 'avatar-leader' : 'avatar-member'}`}
              src={this.getAvatarWithFallback(candidate.user.profile.avatar)}/>
            <a className="row-name-link" onClick={this.linkProfileInfo.bind(this, candidate.user)}>
              {this.getUserNameWithFallback(candidate.user)}
            </a>
          </div>
        )
      }
    }]
    return (
      <div>
        <div className="member-header">
          <h3 className="member-header-label komu-a with-gizmo">{I18N.get('circle.members')}</h3>
        </div>
        <div className="members-list">
          <Table
            className="no-borders cr-table"
            dataSource={members}
            columns={columns}
            bordered={false}
            loading={this.props.loading}
            rowKey="_id"
            pagination={false}
            scroll={{ y: 400 }} />
        </div>
      </div>
    )
  }

  renderTasks() {
    const tasks = this.props.all_tasks
    const clickHandler = !this.props.is_login
      ? this.showLoginRegisterModal.bind(this)
      : this.showTaskModal.bind(this)

    const columns = [{
      title: 'Task',
      key: 'name',
      render: task => {
        return (
          <div key={task._id}>
            <Avatar className="gap-right"
              src={this.getTaskAvatarWithFallback(task.thumbnail)}/>
            <a className="row-name-link" onClick={() => clickHandler(task._id)}>
              {task.name}
            </a>
            {this.taskIsAssigned(task) && <span>&nbsp; - (Assigned)</span>}
          </div>
        )
      }
    }, {
      title: 'Reward',
      key: 'reward',
      width: 150,
      render: task => {
        return task.bidding
          ? 'Bidding'
          : task.reward
            ? task.reward.isUsd
              ? `${task.reward.usd / 100} USD`
              : `${task.reward.ela / 1000} ELA`
            : ''
      }
    }]

    return (
      <div>
        <div className="member-header">
          <h3 className="member-header-label komu-a with-gizmo">{I18N.get('circle.tasks')}</h3>
          {this.isTeamMember() && (
          <Button disabled={true} className="pull-right" onClick={this.createNewTask.bind(this)}>
            {I18N.get('task.createNew')}
          </Button>
          )}
        </div>
        <div className="members-list">
          <Table
            className="no-borders cr-table"
            loading={this.props.task_loading}
            dataSource={tasks}
            columns={columns}
            bordered={false}
            rowKey="_id"
            pagination={false}
            scroll={{ y: 400 }} />
        </div>
      </div>
    )
  }

  /**
     * Returns true if the task is already assigned
     * @param task
     */
  taskIsAssigned(task) {
    return !!_.find(task.candidates, {status: TASK_CANDIDATE_STATUS.APPROVED})
  }

  renderComments() {
    return (
      <div>
        <div className="form-header-wrap">
          <div className="form-header komu-a">
            {this.props.is_login
              ? this.isTeamMember()
                ? I18N.get('circle.createPost')
                : I18N.get('circle.joinToPost')
              : (
                <div>
                  <a className="form-header komu-a" onClick={() => this.showLoginRegisterModal()}>
                    {I18N.get('circle.registerToPost')}
                  </a>
                </div>
              )
            }
          </div>
          <Comments
            headlines={true}
            type="team"
            canPost={this.isTeamMember()}
            returnUrl={`/crcles-detail/${this.props.team.detail._id}`}
            model={this.props.team.detail}/>
        </div>
      </div>
    )
  }

  ord_render () {
    return (
      <div className="c_Circle c_Detail">
        <div className="header">
          <div className="left-box-container">
            <div className="box"/>
          </div>
          <div className="upper-box-container">
            <img src="/assets/images/training_green_slashed_box.png"/>
          </div>
          <div className="right-box-container">
            <div className="small-box"/>
            <div className="box"/>
            <img src="/assets/images/oomph.png"/>
          </div>
          {this.renderHeader()}
        </div>
        <div className="content-section">
          {this.renderContent()}
        </div>
        <div className="tasks-section">
          {this.renderTasks()}
        </div>
        <div className="members-section">
          {this.renderMembers()}
        </div>
        <div className="rectangle double-size pull-right"/>
        <div className="clearfix"/>
        {this.renderComments()}
        <div className="rectangle"/>
        <div className="rectangle double-size"/>
        {this.renderLoginOrRegisterModal()}
        {this.renderTaskModal()}
        {this.renderTaskCreateModal()}
        <Modal
          className="profile-overview-popup-modal"
          visible={!!this.state.showUserInfo}
          onCancel={this.handleCancelProfilePopup.bind(this)}
          footer={null}>
          { this.state.showUserInfo &&
          <ProfilePopup showUserInfo={this.state.showUserInfo}/>
          }
        </Modal>
      </div>
    )
  }

  handleCancelProfilePopup() {
    this.setState({
      showUserInfo: null
    })
  }

  hideShowModal() {
    return () => {
      this.setState({
        showLoginRegisterModal: false
      })
    }
  }

  renderLoginOrRegisterModal() {
    return (
      <Modal
        className="project-detail-nobar"
        visible={this.state.showLoginRegisterModal}
        onOk={this.handleLoginRegisterModalOk}
        onCancel={this.handleLoginRegisterModalCancel}
        footer={null}
        width="70%"
      >
        <div className="login-register-modal">
          <div className="side-image">
            <img src="/assets/images/login-left.png"/>
          </div>
          <div className="side-form">
            <LoginOrRegisterForm onHideModal={this.hideShowModal.bind()}/>
          </div>
        </div>
      </Modal>
    )
  }

  renderTaskModal() {
    return (
      <Modal
        className="project-detail-nobar"
        visible={this.state.showTaskModal}
        onOk={this.handleTaskModalOk}
        onCancel={this.handleTaskModalCancel}
        footer={null}
        width="70%"
      >
        { this.state.showTaskModal &&
        <TaskDetail taskId={this.state.taskDetailId}/>
        }
      </Modal>
    )
  }

  renderTaskCreateModal() {
    return (
      <Modal
        className="project-detail-nobar"
        visible={this.state.showCreateModal}
        onOk={this.handleCreateTaskModalOk}
        onCancel={this.handleCreateTaskModalCancel}
        footer={null}
        width="70%"
      >
        { this.state.showCreateModal && (
        <TaskCreateForm circleId={this.props.match.params.circleId}
                      disableCircleSelect={true}/>
        )}
      </Modal>
    )
  }

    showLoginRegisterModal = () => {
      sessionStorage.setItem('loginRedirect', `/crcles-detail/${this.props.match.params.circleId}`)
      sessionStorage.setItem('registerRedirect', `/crcles-detail/${this.props.match.params.circleId}`)

      this.setState({
        showLoginRegisterModal: true
      })
    }

    showCreateTaskModal = () => {
      this.setState({
        showCreateModal: true
      })
    }

    handleLoginRegisterModalOk = (e) => {
      sessionStorage.removeItem('registerRedirect')

      this.setState({
        showLoginRegisterModal: false
      })
    }

    handleCreateTaskModalOk = (e) => {
      this.setState({
        showCreateModal: false
      })
    }

    handleCreateTaskModalCancel = (e) => {
      this.setState({
        showCreateModal: false
      })
    }

    handleLoginRegisterModalCancel = (e) => {
      sessionStorage.removeItem('registerRedirect')

      this.setState({
        showLoginRegisterModal: false
      })
    }

    handleTaskModalOk = (e) => {
      this.setState({
        showTaskModal: false
      })
    }

    handleTaskModalCancel = (e) => {
      this.setState({
        showTaskModal: false
      })
    }

    showTaskModal(id) {
      this.setState({
        showTaskModal: true,
        taskDetailId: id
      })
    }
}

export default Form.create()(C)
