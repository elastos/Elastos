import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Row,
  Popover,
  Avatar,
  Button,
  Spin,
  Table,
  Form,
  Divider,
  Modal
} from 'antd'
import {
  TASK_CANDIDATE_STATUS,
  TASK_CANDIDATE_TYPE,
  USER_AVATAR_DEFAULT
} from '@/constant'
import LoginOrRegisterForm from '@/module/form/LoginOrRegisterForm/Container'
import ItemsCarousel from 'react-items-carousel'
import _ from 'lodash'
import Application from '../application/Container'
import ApplicationStart from '../application/start/Container'
import ProfilePopup from '@/module/profile/OverviewPopup/Container'
import I18N from '@/I18N'
import './style.scss'

class C extends BaseComponent {
  ord_states() {
    return {
      showLoginRegisterModal: false,
      showApplicationModal: false,
      showApplicationStartModal: false,
      taskCandidateId: null,
      showUserInfo: null
    }
  }

  componentDidMount() {
    const taskId = this.props.taskId
    this.props.getTaskDetail(taskId)

    this.setState({ all_tasks_loading: true })
    this.props.getTasks().then(() => {
      const allTasks = this.getSortedTasks()
      const itemIndex = Math.max(
        _.indexOf(allTasks, _.find(allTasks, { _id: this.props.taskId })),
        0
      )

      this.setState({
        activeSliderItemIndex: itemIndex,
        all_tasks_loading: false
      })
    })

    if (this.props.currentUserId) {
      this.props.getTeams({
        owner: this.props.currentUserId
      })
    }
  }

  componentWillUnmount() {
    this.props.resetTaskDetail()
    this.props.resetAllTeams()
  }

  componentDidUpdate(prevProps) {
    if (prevProps.taskId !== this.props.taskId) {
      this.props.resetTaskDetail()
      this.props.getTaskDetail(this.props.taskId)

      const allTasks = this.getSortedTasks()
      const itemIndex = Math.max(
        _.indexOf(allTasks, _.find(allTasks, { _id: this.props.taskId })),
        0
      )

      // eslint-disable-next-line react/no-did-update-set-state
      this.setState({
        activeSliderItemIndex: itemIndex
      })
    }
  }

  getSortedTasks() {
    return _.sortBy(_.values(this.props.all_tasks), [task => task.dAppId])
  }

  isTaskOwner() {
    return (
      this.props.detail.createdBy &&
      this.props.detail.createdBy._id === this.props.currentUserId
    )
  }

  isUserSubscribed() {
    const curDetail = this.props.detail
    const subscribers = curDetail.subscribers || []
    return !!_.find(subscribers, subscriber => {
      return subscriber.user && subscriber.user._id === this.props.currentUserId
    })
  }

  async applyToProject() {
    const res = await this.props.applyToTask(
      this.props.taskId,
      this.props.currentUserId
    )
    this.showApplicationModal(res._id)
    return res
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
            <img src="/assets/images/login-left.png" />
          </div>
          <div className="side-form">
            <LoginOrRegisterForm onHideModal={this.hideShowModal()} />
          </div>
        </div>
      </Modal>
    )
  }

  renderApplicationModal() {
    return (
      <Modal
        className="project-detail-nobar"
        visible={this.state.showApplicationModal}
        onOk={this.handleApplicationModalOk.bind(this)}
        onCancel={this.handleApplicationModalCancel.bind(this)}
        footer={null}
        width="70%"
      >
        {this.state.taskCandidateId && (
          <Application
            taskId={this.props.taskId}
            taskCandidateId={this.state.taskCandidateId}
          />
        )}
      </Modal>
    )
  }

  renderApplicationStartModal() {
    return (
      <Modal
        className="project-detail-nobar"
        visible={this.state.showApplicationStartModal}
        onOk={this.handleApplicationStartModalOk.bind(this)}
        onCancel={this.handleApplicationStartModalCancel.bind(this)}
        footer={null}
        width="70%"
      >
        {this.state.showApplicationStartModal && (
          <ApplicationStart
            task={this.props.detail}
            finisher={this.handleApplicationStartModalOk.bind(this)}
          />
        )}
      </Modal>
    )
  }

  showLoginRegisterModal = () => {
    sessionStorage.setItem(
      'loginRedirect',
      `/project-detail/${this.props.taskId}`
    )
    sessionStorage.setItem(
      'registerRedirect',
      `/project-detail/${this.props.taskId}`
    )

    this.setState({
      showLoginRegisterModal: true
    })
  }

  showApplicationModal(taskCandidateId) {
    taskCandidateId = taskCandidateId || this.getApplicant()._id
    if (_.isObject(taskCandidateId)) {
      taskCandidateId = this.getApplicant()._id
    }
    this.setState({
      showApplicationModal: true,
      taskCandidateId
    })
  }

  showApplicationStartModal() {
    this.setState({
      showApplicationStartModal: true
    })
  }

  handleLoginRegisterModalOk = e => {
    sessionStorage.removeItem('registerRedirect')

    this.setState({
      showLoginRegisterModal: false
    })
  }

  handleApplicationModalOk = e => {
    this.setState({
      showApplicationModal: false,
      taskCandidateId: null
    })
  }

  handleApplicationStartModalOk = e => {
    this.setState({
      showApplicationStartModal: false
    })
    this.showApplicationModal()
  }

  handleLoginRegisterModalCancel = e => {
    sessionStorage.removeItem('registerRedirect')

    this.setState({
      showLoginRegisterModal: false
    })
  }

  handleApplicationModalCancel() {
    this.setState({
      showApplicationModal: false,
      taskCandidateId: null
    })
  }

  handleApplicationStartModalCancel() {
    this.setState({
      showApplicationStartModal: false
    })
  }

  subscribeToProject() {
    this.props.subscribeToProject(this.props.taskId)
  }

  unsubscribeFromProject() {
    this.props.unsubscribeFromProject(this.props.taskId)
  }

  linkProfileInfo(user) {
    this.setState({
      showUserInfo: user
    })
  }

  linkTeamDetail(teamId) {
    this.props.history.push(`/team-detail/${teamId}`)
  }

  checkForLoading(followup) {
    return this.props.loading ? (
      <div className="valign-wrapper halign-wrapper">
        <Spin size="large" />
      </div>
    ) : (
      _.isFunction(followup) && followup()
    )
  }

  checkForAllTasksLoading(followup) {
    return this.state.all_tasks_loading ? (
      <div className="valign-wrapper halign-wrapper">
        <Spin size="large" />
      </div>
    ) : (
      _.isFunction(followup) && followup()
    )
  }

  isMemberByUserId(userId) {
    const candidate = _.find(this.props.detail.candidates, candidate => {
      if (candidate.type === TASK_CANDIDATE_TYPE.USER) {
        return candidate.user._id === userId
      }
      if (candidate.type === TASK_CANDIDATE_TYPE.TEAM) {
        return candidate.team.owner._id === userId
      }
      return false
    })

    return !!candidate
  }

  isMember(taskCandidateId) {
    const candidate = _.find(this.props.detail.candidates, {
      _id: taskCandidateId
    })
    if (!candidate) {
      return false
    }
    if (candidate.type === TASK_CANDIDATE_TYPE.USER) {
      return candidate.user._id === this.props.currentUserId
    }
    if (candidate.type === TASK_CANDIDATE_TYPE.TEAM) {
      return _.find(
        this.props.ownedTeams,
        item => item._id === candidate.team._id
      )
    }
  }

  getCandidateDisplayName(candidate) {
    const fn = candidate.user.profile.firstName
    const ln = candidate.user.profile.lastName
    const un = candidate.user.username

    return _.isEmpty(fn) && _.isEmpty(ln) ? un : [fn, ln].join(' ')
  }

  getAvatarWithFallback(avatar) {
    return _.isEmpty(avatar) ? USER_AVATAR_DEFAULT : avatar
  }

  getCandidateAvatar(candidate) {
    const avatar = candidate.user.profile.avatar
    return this.getAvatarWithFallback(avatar)
  }

  getTeamAvatar(candidate) {
    const avatar =
      candidate.team &&
      !_.isEmpty(candidate.team.pictures) &&
      candidate.team.pictures[0].url
    return this.getAvatarWithFallback(avatar)
  }

  getCurrentContributors() {
    const detail = this.props.detail
    const applicants = _.filter(detail.candidates, {
      status: TASK_CANDIDATE_STATUS.APPROVED
    })
    if (this.props.detail.createdBy) {
      applicants.unshift({
        _id: 'such_fake_id',
        user: this.props.detail.createdBy,
        type: TASK_CANDIDATE_TYPE.USER
      })
    }

    const columns = [
      {
        title: 'Name',
        key: '_id',
        render: candidate => {
          return (
            <div>
              {candidate.type === TASK_CANDIDATE_TYPE.USER && (
                <div>
                  <a onClick={this.linkProfileInfo.bind(this, candidate.user)}>
                    <Avatar
                      className={`gap-right ${
                        candidate._id === 'such_fake_id'
                          ? 'avatar-leader'
                          : 'avatar-member'
                      }`}
                      src={this.getCandidateAvatar(candidate)}
                    />
                    {this.getCandidateDisplayName(candidate)}
                  </a>
                </div>
              )}
              {candidate.type === TASK_CANDIDATE_TYPE.TEAM && (
                <div>
                  <a
                    onClick={this.linkTeamDetail.bind(this, candidate.team._id)}
                  >
                    <Avatar
                      className="gap-right"
                      src={this.getTeamAvatar(candidate)}
                    />
                    {candidate.team.name}
                  </a>
                </div>
              )}
            </div>
          )
        }
      },
      {
        title: 'Action',
        render: candidate => {
          return (
            <div>
              {this.isTaskOwner() && candidate._id !== 'such_fake_id' && (
                <div className="text-right">
                  <a onClick={this.removeUser.bind(this, candidate._id)}>
                    {I18N.get('project.detail.remove')}
                  </a>
                </div>
              )}
            </div>
          )
        }
      }
    ]

    return (
      <Table
        loading={this.props.loading}
        className="no-borders headerless"
        dataSource={applicants}
        columns={columns}
        bordered={false}
        pagination={false}
        rowKey="_id"
      />
    )
  }

  approveUser(taskCandidateId) {
    this.props.acceptCandidate(taskCandidateId)
  }

  disapproveUser(taskCandidateId) {
    this.props.rejectCandidate(taskCandidateId)
  }

  withdrawApplication(taskCandidateId) {
    this.props.withdrawCandidate(taskCandidateId)
  }

  removeUser(taskCandidateId) {
    this.props.rejectCandidate(taskCandidateId)
  }

  removeUserByUserId(userId) {
    const candidate = _.find(
      this.props.detail.candidates,
      candidate => candidate.user._id === userId
    )
    if (!candidate) {
      return false
    }
    return this.removeUser(candidate._id)
  }

  getNumberOfLikes() {
    return _.size(this.props.detail.subscribers)
  }

  getCurrentSubscribers() {
    const subscribers = this.props.detail.subscribers
    const columns = [
      {
        title: 'Name',
        key: 'user',
        render: candidate => {
          return (
            <div>
              <div>
                <a onClick={this.linkProfileInfo.bind(this, candidate.user)}>
                  <Avatar
                    className="gap-right"
                    src={this.getCandidateAvatar(candidate)}
                  />
                  {this.getCandidateDisplayName(candidate)}
                </a>
              </div>
            </div>
          )
        }
      }
    ]

    return (
      <Table
        loading={this.props.loading}
        className="no-borders headerless"
        dataSource={subscribers}
        columns={columns}
        bordered={false}
        pagination={false}
        rowKey="_id"
      />
    )
  }

  getCurrentApplicantsData() {
    const detail = this.props.detail
    return _.filter(detail.candidates, {
      status: TASK_CANDIDATE_STATUS.PENDING
    })
  }

  getCurrentApplicants() {
    const applicants = this.getCurrentApplicantsData()
    const columns = [
      {
        title: I18N.get('profile.detail.table.name'),
        key: 'user',
        render: candidate => {
          return (
            <div>
              {candidate.type === TASK_CANDIDATE_TYPE.USER && (
                <div>
                  <a onClick={this.linkProfileInfo.bind(this, candidate.user)}>
                    <Avatar
                      className="gap-right"
                      src={this.getCandidateAvatar(candidate)}
                    />
                    {this.getCandidateDisplayName(candidate)}
                  </a>
                </div>
              )}
              {candidate.type === TASK_CANDIDATE_TYPE.TEAM && (
                <div>
                  <a
                    onClick={this.linkTeamDetail.bind(this, candidate.team._id)}
                  >
                    <Avatar
                      className="gap-right"
                      src={this.getTeamAvatar(candidate)}
                    />
                    {candidate.team.name}
                  </a>
                </div>
              )}
            </div>
          )
        }
      },
      {
        title: I18N.get('profile.detail.table.action'),
        key: 'action',
        render: candidate => {
          return (
            <div className="text-right">
              {(this.props.page === 'ADMIN' ||
                this.isTaskOwner() ||
                this.isMember(candidate._id)) && (
                <span>
                  <a
                    onClick={this.showApplicationModal.bind(
                      this,
                      candidate._id
                    )}
                  >
                    {I18N.get('project.detail.view')}
                  </a>
                  <Divider type="vertical" />
                </span>
              )}
              {this.isMember(candidate._id) && (
                <span>
                  <a
                    onClick={this.withdrawApplication.bind(this, candidate._id)}
                  >
                    {I18N.get('project.detail.withdraw_application')}
                  </a>
                  {this.isTaskOwner() && <Divider type="vertical" />}
                </span>
              )}
              {this.isTaskOwner() && (
                <span className="inline-block">
                  <a onClick={this.approveUser.bind(this, candidate._id)}>
                    {I18N.get('project.detail.approve')}
                  </a>
                  <Divider type="vertical" />
                  <a onClick={this.disapproveUser.bind(this, candidate._id)}>
                    {I18N.get('project.detail.disapprove')}
                  </a>
                </span>
              )}
            </div>
          )
        }
      }
    ]

    return (
      <Table
        loading={this.props.loading}
        className="no-borders headerless"
        dataSource={applicants}
        columns={columns}
        bordered={false}
        pagination={false}
        rowKey="_id"
      />
    )
  }

  getApplicant() {
    return (
      !_.isEmpty(this.props.detail.candidates) &&
      this.props.detail.candidates.find(candidate => {
        return (
          (candidate.user && candidate.user._id === this.props.currentUserId) ||
          (candidate.team &&
            candidate.team.owner._id === this.props.currentUserId)
        )
      })
    )
  }

  getActions() {
    const likeHandler = this.props.is_login
      ? this.isUserSubscribed()
        ? this.unsubscribeFromProject
        : this.subscribeToProject
      : this.showLoginRegisterModal

    const applyHandler = this.props.is_login
      ? this.isMemberByUserId(this.props.currentUserId)
        ? this.showApplicationModal
        : this.showApplicationStartModal
      : this.showLoginRegisterModal

    return (
      <div className="halign-wrapper valign-wrapper action-wrapper">
        <div>
          <Button
            loading={this.props.loading}
            icon={this.isUserSubscribed() ? 'dislike' : 'like'}
            onClick={likeHandler.bind(this)}
          >
            {this.isUserSubscribed()
              ? I18N.get('pdetail.unlike')
              : I18N.get('pdetail.like')}
          </Button>
          <Button
            disabled={true}
            loading={this.props.loading}
            icon="message"
            onClick={applyHandler.bind(this)}
            /* disabled={this.isTaskOwner()} */
          >
            {I18N.get('pdetail.involve')}
          </Button>
        </div>
      </div>
    )
  }

  linkToProjectTask(id) {
    this.props.history.replace(`/project-detail/${id}`)
  }

  getProjectSlider() {
    const all_tasks = this.getSortedTasks()
    const projects = _.map(all_tasks, (task, id) => {
      const isActive = task._id === this.props.taskId
      return (
        <div
          key={task.dAppId}
          className="halign-wrapper full-width full-height"
        >
          <div className="slider-project-icon">
            <div
              className={`project-icon ${isActive ? 'active' : ''}`}
              onClick={this.linkToProjectTask.bind(this, task._id)}
            >
              <span className="project-number komu-a">{task.dAppId}</span>
              <div className="base-icon" />
              <img className="project-avatar" src={task.thumbnail} />
            </div>
            <div
              className={`project-name ${isActive ? 'active' : ''}`}
              onClick={this.linkToProjectTask.bind(this, task._id)}
            >
              {task.name}
            </div>
          </div>
        </div>
      )
    })

    return this.checkForAllTasksLoading(() => (
      <ItemsCarousel
        numberOfCards={7}
        slidesToScroll={3}
        firstAndLastGutter={true}
        gutter={12}
        requestToChangeActive={this.changeActiveSliderItem.bind(this)}
        activeItemIndex={this.state.activeSliderItemIndex}
        activePosition="center"
        chevronWidth={12}
        outsideChevron={true}
        leftChevron={
          <img className="arrow mirror-image" src="/assets/images/arrow.svg" />
        }
        rightChevron={<img className="arrow" src="/assets/images/arrow.svg" />}
      >
        {projects}
      </ItemsCarousel>
    ))
  }

  changeActiveSliderItem(index) {
    this.setState({
      activeSliderItemIndex: index
    })
  }

  ord_render() {
    const isMember = this.isMemberByUserId(this.props.currentUserId)

    return (
      <div>
        <div className="ebp-wrap project-slider">{this.getProjectSlider()}</div>
        <div className="c_Project c_Detail">
          <div className="project-header-additions">
            <div className="left-box-container">
              <div className="box" />
              <img src="/assets/images/training_circle.png" />
            </div>
            <div className="right-box-container">
              <div className="small-box" />
              <div className="box" />
              <img src="/assets/images/oomph.png" />
            </div>
          </div>
          <div className="ebp-wrap">{this.getHeader()}</div>
          <div className="project-info">
            {(this.props.is_admin ||
              this.isTaskOwner() ||
              this.props.page === 'PUBLIC') && (
              <Row className="applications ebp-wrap">
                <h3 className="no-margin with-gizmo">
                  {this.props.detail.bidding
                    ? I18N.get('project.detail.pending_bids')
                    : I18N.get('project.detail.pending_applications')}
                </h3>
                {this.getCurrentApplicantsData().length ? (
                  this.getCurrentApplicants()
                ) : (
                  <div className="no-data">
                    {I18N.get('profile.detail.noapplications')}
                  </div>
                )}
              </Row>
            )}

            {(this.props.is_admin ||
              this.isTaskOwner() ||
              this.props.page === 'PUBLIC') && (
              <Row className="subscribers ebp-wrap">
                <h3 className="no-margin with-gizmo">
                  {I18N.get('project.detail.subscribers')}
                </h3>
                {this.getCurrentSubscribers()}
              </Row>
            )}
          </div>
          <div className="rectangle" />
          <div className="project-description-1">{this.getDescription1()}</div>
          <div className="rectangle pull-right" />
          <div className="project-description-2">{this.getDescription2()}</div>
          <div className="project-application">{this.getApplication()}</div>
          <div className="rectangle" />
          <div className="rectangle rectangle-long" />
          {this.renderLoginOrRegisterModal()}
          {this.renderApplicationModal()}
          {this.renderApplicationStartModal()}
          <Modal
            className="profile-overview-popup-modal"
            visible={!!this.state.showUserInfo}
            onCancel={this.handleCancelProfilePopup.bind(this)}
            footer={null}
          >
            {this.state.showUserInfo && (
              <ProfilePopup showUserInfo={this.state.showUserInfo} />
            )}
          </Modal>
        </div>
      </div>
    )
  }

  handleCancelProfilePopup() {
    this.setState({
      showUserInfo: null
    })
  }

  getHeader() {
    const project = _.find(_.values(this.props.all_tasks), {
      _id: this.props.taskId
    })
    const dAppId = project ? `#${project.dAppId}-` : ''

    return (
      <div className="project-header">
        <div className="rect-container">
          <div className="rect" />
        </div>
        <div className="project-name komu-a">
          {dAppId} {this.props.detail.name}
        </div>
        <div className="strike-text project-funding">
          <div className="strike-line" />
          <p>{I18N.get('profile.detail.finding')}</p>
        </div>
        <div className="actions">{this.getActions()}</div>
      </div>
    )
  }

  getDescription1() {
    return (
      <div className="ebp-wrap">
        <h3 className="with-gizmo">
          {I18N.get('developer.cr100.pitch.problem')}
        </h3>
        <div>{this.props.detail.pitch && this.props.detail.pitch.problem}</div>

        <h3 className="with-gizmo">
          {I18N.get('developer.cr100.pitch.valueProposition')}
        </h3>
        <div>
          {this.props.detail.pitch && this.props.detail.pitch.valueProposition}
        </div>

        <h3 className="with-gizmo">
          {I18N.get('developer.cr100.pitch.useCase')}
        </h3>
        <div>{this.props.detail.pitch && this.props.detail.pitch.useCase}</div>
      </div>
    )
  }

  getDescription2() {
    return (
      <div className="ebp-wrap">
        <h3 className="with-gizmo">
          {I18N.get('developer.cr100.pitch.beneficiaries')}
        </h3>
        <div>
          {this.props.detail.pitch &&
            this.format(this.props.detail.pitch.beneficiaries)}
        </div>

        <h3 className="with-gizmo">
          {I18N.get('developer.cr100.pitch.elaInfrastructure')}
        </h3>
        <div>
          {this.props.detail.pitch &&
            this.format(this.props.detail.pitch.elaInfrastructure)}
        </div>
      </div>
    )
  }

  getApplication() {
    const applyHandler = this.props.is_login
      ? this.getApplicant()
        ? this.showApplicationModal
        : this.showApplicationStartModal
      : this.showLoginRegisterModal
    return (
      <div className="ebp-wrap">
        <div className="rect-container">
          <div className="rect" />
        </div>
        <Popover
          content={I18N.get('notice.suspended')}
          title={I18N.get('.suspended')}
        >
          <Button className="apply-button">{I18N.get('.apply')}</Button>
        </Popover>
      </div>
    )
  }

  format(contents = '') {
    const elements = _.map(contents, (char, ind) =>
      char === '-' ? <br key={ind} /> : char
    )
    return elements
  }
}

export default Form.create()(C)
