import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import moment from 'moment'
import {
  Col,
  Row,
  Tag,
  Icon,
  Carousel,
  Avatar,
  Button,
  Spin,
  Table,
  Form,
  Divider,
  Modal,
  Popover
} from 'antd'
import I18N from '@/I18N'
import {
  TASK_CANDIDATE_STATUS,
  TASK_CANDIDATE_TYPE,
  TASK_STATUS,
  USER_AVATAR_DEFAULT,
  TEAM_AVATAR_DEFAULT,
  TASK_TYPE
} from '@/constant'
import Comments from '@/module/common/comments/Container'
import ProjectApplication from '@/module/project/application/Container'
import ProjectApplicationStart from '@/module/page/project_detail/application/start/Container'
import ProfilePopup from '@/module/profile/OverviewPopup/Container'
import _ from 'lodash'
import { getSafeUrl } from '@/util/url'
import MarkdownPreview from '@/module/common/MarkdownPreview'

import './style.scss'

/*
 * Project Pop-up UI
 *
 */
class C extends BaseComponent {
  ord_states() {
    return {
      showAppModal: false,
      showApplicationStartModal: false,
      projectCandidateId: null,
      showUserInfo: null
    }
  }

  componentDidMount() {
    const taskId = this.props.taskId
    this.props.getTaskDetail(taskId)

    // This sets both user.teams and teams.all_teams
    this.props.getTeams({ owner: this.props.currentUserId })
  }

  componentWillUnmount() {
    this.props.resetTaskDetail()
  }

  /**
   * Not used, for bidding projects we hide comments anyways, and otherwise it should just be a public comments thread
   */
  canComment() {
    const isTaskCandidate = _.find(this.props.task.candidates, candidate => {
      return (
        candidate.user &&
        candidate.user._id === this.props.currentUserId &&
        candidate.status === TASK_CANDIDATE_STATUS.APPROVED
      )
    })

    const allCandidateTeamIds = _.compact(
      _.map(this.props.task.candidates, candidate => {
        return candidate.team && candidate.team._id
      })
    )

    const currentUserTeamIds = _.map(this.props.ownedTeams, '_id')
    const belongsToMemberTeam = !_.isEmpty(
      _.intersection(allCandidateTeamIds, currentUserTeamIds)
    )
    const isTaskOwner =
      this.props.task.createdBy &&
      this.props.task.createdBy._id === this.props.currentUserId

    return isTaskCandidate || belongsToMemberTeam || isTaskOwner
  }

  renderHeader() {
    return (
      <div className="header">
        <h3 className="with-gizmo">{this.props.task.name}</h3>
      </div>
    )
  }

  renderMeta() {
    const generateRow = (key, value, cssRowClass) => (
      <Row className={[cssRowClass, 'meta-row'].join(' ')}>
        <Col span={8}>{key}</Col>
        <Col span={16}>{value}</Col>
      </Row>
    )

    const generateHtmlRow = (key, value, cssRowClass) => (
      <Row className={[cssRowClass, 'meta-row'].join(' ')}>
        <Col span={8}>{key}</Col>
        <Col span={16}>
          <MarkdownPreview content={value} />
        </Col>
      </Row>
    )

    const detail = this.props.task
    const budget = this.getBudgetFormatted()
    const reward = this.getRewardFormatted()
    const EVENT_DATE_FORMAT = 'MMM D, YYYY - HH:mm'
    const DEADLINE_FORMAT = 'MMM D'

    return (
      <div className="meta">
        {generateRow(
          I18N.get('task.owner'),
          <a onClick={this.linkProfileInfo.bind(this, detail.createdBy)}>
            {this.getUserNameWithFallback(detail.createdBy)}
          </a>
        )}

        {detail.circle &&
          generateRow(
            I18N.get('task.circle'),
            I18N.get(`crcle.${detail.circle.name.toLowerCase()}`)
          )}

        {generateRow(
          I18N.get('task.type'),
          I18N.get(`developer.search.${detail.type.toLowerCase()}`)
        )}

        {generateRow(
          I18N.get('task.category'),
          I18N.get(`taks.application.${detail.category.toLowerCase()}`)
        )}

        {detail.location &&
          generateRow(I18N.get('task.location'), detail.location)}

        {detail.community &&
          generateRow(I18N.get('task.community'), this.getCommunityDisp())}

        {detail.approvedBy &&
          generateRow(
            I18N.get('task.approvedBy'),
            <div>
              {detail.approvedBy.username}
              {detail.approvedDate && (
                <span>
                  {I18N.get('project.admin.statusHelp.approvedOn')}{' '}
                  {moment(detail.approvedDate).format('MMM D, YYYY')}
                </span>
              )}
            </div>
          )}

        {detail.applicationDeadline &&
          generateRow(
            I18N.get('task.applyDeadline'),
            moment(detail.applicationDeadline).format(DEADLINE_FORMAT)
          )}

        {detail.completionDeadline &&
          generateRow(
            I18N.get('task.completionDeadline'),
            moment(detail.completionDeadline).format(DEADLINE_FORMAT)
          )}

        {detail.bidding &&
          generateRow(
            I18N.get('task.referenceBid'),
            detail.referenceBid || I18N.get('task.referenceBid.none')
          )}

        {(!detail.bidding &&
          budget &&
          generateRow(
            I18N.get('task.budget'),
            <div>
              <span>{budget}</span>
              {this.getBudgetExplanation()}
            </div>
          )) ||
          null}

        {(!detail.bidding &&
          reward &&
          generateRow(
            I18N.get('task.reward'),
            <div>
              <span>{reward}</span>
              {this.getRewardExplanation()}
            </div>
          )) ||
          null}

        {detail.goals &&
          generateRow(I18N.get('task.goals'), detail.goals, 'task-goals')}

        {detail.descBreakdown &&
          generateHtmlRow(
            I18N.get('task.descBreakdown'),
            detail.descBreakdown,
            'task-breakdown'
          )}

        {detail.eventDateRangeStart &&
          generateRow(
            I18N.get('task.eventStart'),
            `${moment(detail.eventDateRangeStart).format(EVENT_DATE_FORMAT)} (${
              detail.eventDateStatus
            })`
          )}

        {detail.eventDateRangeEnd &&
          generateRow(
            I18N.get('task.eventEnd'),
            moment(detail.eventDateRangeEnd).format(EVENT_DATE_FORMAT)
          )}

        {generateHtmlRow(
          I18N.get('task.description'),
          detail.description,
          'task-description'
        )}

        {detail.attachment &&
          generateRow(
            I18N.get('task.attachment'),
            <a href={getSafeUrl(detail.attachment)} target="_blank">
              {detail.attachmentFilename}
            </a>
          )}

        {detail.infoLink &&
          generateRow(
            I18N.get('task.infoLink'),
            <a href={getSafeUrl(detail.infoLink)} target="_blank">
              {detail.infoLink}
            </a>
          )}
      </div>
    )
  }

  /**
   * Assignment Info
   *
   * - based on the assignSelf boolean field
   *
   * This refers to the next action after a task approval
   * - If assignSelf = true, it is a private task which means it will be assigned to the owner after approval
   * - If assignSelf = false, it is public and after it will be open for applications or bidding if (bidding = true)
   */
  renderAssignmentInfo() {
    return (
      <div>
        {this.props.task.assignSelf ? (
          <div className="assignment-info">
            <Tag>Assignment Type: PRIVATE</Tag>
            <Popover
              content={`After APPROVAL this ${I18N.get(
                `taskType.${this.props.task.type}`
              )} is assigned to the owner`}
            >
              <Icon className="help-icon" type="question-circle-o" />
            </Popover>
          </div>
        ) : (
          <div className="assignment-info">
            <Tag>Assignment Type: PUBLIC</Tag>
            <Popover
              content={`After APPROVAL this ${I18N.get(
                `taskType.${this.props.task.type}`
              )} becomes publicly available`}
            >
              <Icon className="help-icon" type="question-circle-o" />
            </Popover>
          </div>
        )}
      </div>
    )
  }

  ord_render() {
    const detail = this.props.task
    const loading = _.isEmpty(detail)
    const isTaskOwner =
      this.props.task.createdBy &&
      this.props.task.createdBy._id === this.props.currentUserId

    return (
      <div className="c_Project c_Detail">
        {loading ? (
          <div className="full-width full-height valign-wrapper halign-wrapper">
            <Spin className="loading-spinner" />
          </div>
        ) : (
          <div className="detail-container">
            {this.getImageCarousel()}
            {this.renderHeader()}
            {this.props.task.status === TASK_STATUS.PENDING &&
              this.renderAssignmentInfo()}
            {this.renderMeta()}

            {/*
             * Apply Button
             * - this may be unintuitive but we should always show the button,
             *   you can always apply as a team or user
             * unless you've exhausted all the teams, but even then we can inform
             *   the user of this in a better way than hiding
             */}
            {this.props.page !== 'LEADER' &&
              !this.props.is_admin &&
              !isTaskOwner &&
              this.renderApplyButton()}

            {/*
             * Approved Applicants (might not be necessary except for admin/leader)
             */}
            {this.getCurrentContributorsData().length
              ? this.renderContributors()
              : ''}

            {/*
             * Pending Bids / Applications - only show if CREATED/PENDING
             */}
            {this.renderPendingCandidates()}

            {/*
             * Comments
             * - not enabled for bidding projects to minimize confusion in a closed bid
             */}
            {!detail.bidding ? (
              <Row>
                <br />
                <Comments
                  type="task"
                  canPost={true}
                  canSubscribe={!isTaskOwner}
                  model={this.props.taskId}
                  returnUrl={`/task-detail/${this.props.taskId}`}
                />
              </Row>
            ) : (
              <Row>
                <Col className="center">
                  <br />
                  <br />
                  <span className="no-info">
                    {I18N.get('project.detail.comments_disabled')}
                  </span>
                </Col>
              </Row>
            )}
          </div>
        )}
        {this.renderApplicationStartModal()}
        {this.renderViewApplicationModal()}
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
    )
  }

  handleCancelProfilePopup() {
    this.setState({
      showUserInfo: null
    })
  }

  renderViewApplicationModal() {
    return (
      <Modal
        className="project-detail-nobar no-modal-padding"
        visible={this.state.showAppModal}
        onOk={this.handleAppModalOk}
        onCancel={this.handleAppModalCancel}
        footer={null}
        width="70%"
      >
        <ProjectApplication applicantId={this.state.projectCandidateId} />
      </Modal>
    )
  }

  renderApplyButton() {
    const detail = this.props.task

    // if not bidding check if there is already an approved
    if (
      !detail.bidding &&
      _.find(
        detail.candidates,
        candidate => candidate.status === TASK_CANDIDATE_STATUS.APPROVED
      )
    ) {
      return
    }

    // for bidding we must be in PENDING
    if (
      detail.bidding &&
      _.indexOf([TASK_STATUS.CREATED, TASK_STATUS.PENDING], detail.status) < 0
    ) {
      return
    }

    return (
      <Row className="actions">
        <Button
          type="primary"
          onClick={() => this.showApplicationStartModal()}
          disabled={!this.canApply()}
        >
          {detail.bidding
            ? detail.type === TASK_TYPE.TASK || detail.type === TASK_TYPE.EVENT
              ? I18N.get('project.detail.popup.bid_task')
              : I18N.get('project.detail.popup.bid_project')
            : detail.type === TASK_TYPE.TASK || detail.type === TASK_TYPE.EVENT
            ? I18N.get('project.detail.popup.join_task')
            : I18N.get('project.detail.popup.join_project')}
        </Button>
      </Row>
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
          <ProjectApplicationStart
            task={this.props.task}
            finisher={this.handleApplicationStartModalOk.bind(this)}
          />
        )}
      </Modal>
    )
  }

  /**
   * Render pending bids or applications
   *
   * BIDDING - we show for CREATED/PENDING status only
   *
   * For Admins - only admins can create tasks/projects for bidding
   * - they can see all applications (user/team), we assume they are never a bidder
   *
   * For everyone else they can only see their own bids and the total number of bids
   *
   *
   * PROJECT - we show if APPROVED, but no one is selected yet
   *
   * We can see other people who applied
   */
  renderPendingCandidates() {
    const currentUserId = this.props.currentUserId
    const detail = this.props.task

    // status checks
    if (
      detail.bidding &&
      _.indexOf(
        [TASK_STATUS.CREATED, TASK_STATUS.PENDING, TASK_STATUS.APPROVED],
        detail.status
      ) < 0
    ) {
      return ''
    }

    if (
      !detail.bidding &&
      _.find(
        detail.candidates,
        candidate => candidate.status === TASK_CANDIDATE_STATUS.APPROVED
      )
    ) {
      return ''
    }

    let pendingCandidates = this.getPendingCandidates()
    const pendingCandidatesCnt = pendingCandidates.length

    // only show current user's bids if it's bidding - for projects with a set reward we can show them
    if (!this.props.is_admin && detail.bidding) {
      pendingCandidates = _.filter(pendingCandidates, candidate => {
        if (
          candidate.type === TASK_CANDIDATE_TYPE.USER &&
          candidate.user._id === currentUserId
        ) {
          return true
        }

        if (
          candidate.type === TASK_CANDIDATE_TYPE.TEAM &&
          this.loggedInUserBelongsToCandidate(candidate)
        ) {
          // here we make the assumption that any member of a team can view the team's bid
          return true
        }
      })
    }

    const title = detail.bidding
      ? this.props.is_admin
        ? I18N.get('project.detail.pending_bids')
        : I18N.get('project.detail.your_bids')
      : I18N.get('project.detail.pending_applications')
    const bidsLeft = [
      I18N.get('project.detail.bidding_cur_1'),
      pendingCandidatesCnt - pendingCandidates.length,
      I18N.get('project.detail.bidding_cur_2')
    ].join(' ')

    return (
      <Row className="applications">
        <h3 className="no-margin title with-gizmo">{title}</h3>

        {pendingCandidates.length && this.renderCandidates(pendingCandidates)}

        {/* this works because we filtered pendingCandidates after we saved the count */}
        {(this.props.page !== 'ADMIN' || !this.props.is_admin) &&
          this.props.task.createdBy !== this.props.currentUserId &&
          detail.bidding &&
          bidsLeft}

        {!detail.bidding && pendingCandidates.length === 0 && (
          <div className="no-data no-info">
            {I18N.get('project.detail.noapplications')}
          </div>
        )}
      </Row>
    )
  }

  getAvatarWithFallback(avatar) {
    return _.isEmpty(avatar) ? USER_AVATAR_DEFAULT : avatar
  }

  getTeamAvatarWithFallback(avatar) {
    return _.isEmpty(avatar) ? TEAM_AVATAR_DEFAULT : avatar
  }

  getUserNameWithFallback(user) {
    if (_.isEmpty(user.profile.firstName) && _.isEmpty(user.profile.lastName)) {
      return user.username
    }

    return _.trim([user.profile.firstName, user.profile.lastName].join(' '))
  }

  renderCandidates(candidates) {
    const columns = [
      {
        title: I18N.get('project.detail.columns.name'),
        key: 'name',
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
                      src={this.getAvatarWithFallback(
                        candidate.user.profile.avatar
                      )}
                    />
                    {this.getUserNameWithFallback(candidate.user)}
                  </a>
                </div>
              )}
              {candidate.type === TASK_CANDIDATE_TYPE.TEAM && (
                <div>
                  <a onClick={this.linkTeamInfo.bind(this, candidate.team._id)}>
                    <Avatar
                      className="gap-right"
                      src={this.getTeamAvatarWithFallback(
                        !_.isEmpty(candidate.team.pictures) &&
                          candidate.team.pictures[0].url
                      )}
                    />
                    {candidate.team.name}
                    {this.loggedInUserOwnerOfCandidate(candidate) ? (
                      <span className="no-info">
                        {' '}
                        ({I18N.get('task.owner')})
                      </span>
                    ) : (
                      <span className="no-info">
                        {' '}
                        ({I18N.get('role.member')})
                      </span>
                    )}
                  </a>
                </div>
              )}
            </div>
          )
        }
      },
      {
        title: I18N.get('project.detail.columns.action'),
        key: 'action',
        render: candidate => {
          return (
            <div className="text-right">
              {this.props.loading ? (
                <Spin />
              ) : (
                <div>
                  {this.props.task.bidding && (
                    <span>Bid: {candidate.bid} ELA</span>
                  )}
                  {(this.props.page === 'ADMIN' ||
                    this.isTaskOwner() ||
                    this.props.is_admin ||
                    this.loggedInUserBelongsToCandidate(candidate)) && (
                    <span>
                      <Divider type="vertical" />
                      <a onClick={this.showAppModal.bind(this, candidate._id)}>
                        {I18N.get('project.detail.view')}
                      </a>
                    </span>
                  )}
                  {this.loggedInUserOwnerOfCandidate(candidate) && (
                    <span>
                      <Divider type="vertical" />
                      <a
                        onClick={this.withdrawApplication.bind(
                          this,
                          candidate._id
                        )}
                      >
                        {I18N.get('project.detail.withdraw_application')}
                      </a>
                    </span>
                  )}
                  {(this.isTaskOwner() || this.props.is_admin) && (
                    <span className="inline-block">
                      {candidate.status !== TASK_STATUS.APPROVED && (
                        <span>
                          <Divider type="vertical" />
                          <a
                            onClick={this.approveUser.bind(this, candidate._id)}
                          >
                            {I18N.get('project.detail.approve')}
                          </a>
                          <Divider type="vertical" />
                        </span>
                      )}
                      <a
                        onClick={this.disapproveUser.bind(this, candidate._id)}
                      >
                        {I18N.get('project.detail.disapprove')}
                      </a>
                    </span>
                  )}
                </div>
              )}
            </div>
          )
        }
      }
    ]

    return (
      <Table
        className="no-borders headerless"
        dataSource={candidates}
        columns={columns}
        bordered={false}
        pagination={false}
        rowKey="_id"
      />
    )
  }

  /**
   * For bidding tasks, contributors are the actual assigned user
   */
  renderContributors() {
    const currentContributors = this.getCurrentContributorsData()

    const columns = [
      {
        title: I18N.get('project.detail.columns.name'),
        key: 'name',
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
                      src={this.getAvatarWithFallback(
                        candidate.user.profile.avatar
                      )}
                    />
                    {this.getUserNameWithFallback(candidate.user)}
                  </a>
                </div>
              )}
              {candidate.type === TASK_CANDIDATE_TYPE.TEAM && (
                <div>
                  <a onClick={this.linkTeamInfo.bind(this, candidate.team._id)}>
                    <Avatar
                      className="gap-right"
                      src={this.getAvatarWithFallback(
                        !_.isEmpty(candidate.team.pictures) &&
                          candidate.team.pictures[0].url
                      )}
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
        title: I18N.get('project.detail.columns.action'),
        key: 'action',
        render: candidate => {
          return (
            <div className="text-right">
              {(this.isTaskOwner() || this.props.is_admin) &&
                !this.props.task.bidding && (
                  <a onClick={this.removeUser.bind(this, candidate._id)}>
                    {I18N.get('project.detail.remove')}
                  </a>
                )}
              {(this.isTaskOwner() || this.props.is_admin) &&
                this.props.task.bidding && (
                  <span>Bid: {candidate.bid} ELA</span>
                )}
              {(this.isTaskOwner() ||
                this.props.is_admin ||
                this.loggedInUserBelongsToCandidate(candidate)) && (
                <span>
                  <Divider type="vertical" />
                  <a onClick={this.showAppModal.bind(this, candidate._id)}>
                    {I18N.get('project.detail.view')}
                  </a>
                </span>
              )}
            </div>
          )
        }
      }
    ]

    return (
      <Row className="contributors">
        <h3 className="no-margin align-left with-gizmo">
          {this.props.task.bidding
            ? I18N.get('project.detail.bidding_winner')
            : I18N.get('project.detail.current_contributors')}
        </h3>

        <Table
          className="no-borders headerless"
          dataSource={currentContributors}
          columns={columns}
          bordered={false}
          pagination={false}
          rowKey="_id"
        />
      </Row>
    )
  }

  /*
   ****************************************************************************************************************
   * Helpers
   ****************************************************************************************************************
   */
  isUnapproved() {
    return (
      this.props.task.status === TASK_STATUS.CREATED ||
      this.props.task.status === TASK_STATUS.PENDING
    )
  }

  loggedInUserBelongsToCandidate(candidate) {
    const loggedInUserId = this.props.currentUserId
    if (
      candidate.type === TASK_CANDIDATE_TYPE.USER &&
      candidate.user._id === loggedInUserId
    ) {
      return true
    }

    if (
      candidate.type === TASK_CANDIDATE_TYPE.TEAM &&
      _.find(candidate.team.members, { user: loggedInUserId })
    ) {
      return true
    }
  }

  /**
   * Is the logged in user the passed in candidate
   * or the owner of the team
   *
   * @param candidate
   * @return Boolean
   */
  loggedInUserOwnerOfCandidate(candidate) {
    const loggedInUserId = this.props.currentUserId
    if (
      candidate.type === TASK_CANDIDATE_TYPE.USER &&
      candidate.user._id === loggedInUserId
    ) {
      return true
    }

    if (
      candidate.type === TASK_CANDIDATE_TYPE.TEAM &&
      candidate.team.owner._id === loggedInUserId
    ) {
      return true
    }
  }

  // check if logged in user has applied by themselves
  hasAppliedBySelf() {
    const loggedInUserId = this.props.currentUserId
    const pendingCandidates = this.getPendingCandidates()
    return !!_.find(
      pendingCandidates,
      candidate =>
        candidate.type === TASK_CANDIDATE_TYPE.USER &&
        candidate.user._id === loggedInUserId
    )
  }

  // check if logged in user has applied by the passed in team
  hasAppliedByTeam(team) {
    const pendingCandidates = this.getPendingCandidates()
    return !!_.find(pendingCandidates, candidate => {
      return (
        candidate.type === TASK_CANDIDATE_TYPE.TEAM &&
        candidate.team._id === team._id
      )
    })
  }

  /*
   ****************************************************************************************************************
   * Modals
   ****************************************************************************************************************
   */
  showAppModal = projectCandidateId => {
    this.setState({
      showAppModal: true,
      projectCandidateId
    })
  }

  handleAppModalOk = e => {
    this.setState({
      showAppModal: false
    })
  }

  handleAppModalCancel = e => {
    this.setState({
      showAppModal: false
    })
  }

  getCurrency() {
    return 'USD'
  }

  getReward() {
    if (!this.props.task.reward) {
      return null
    }

    return this.props.task.reward ? this.props.task.reward.usd / 100 : null
  }

  getRewardElaPerUsd() {
    return this.props.task.reward && this.props.task.reward.elaPerUsd
  }

  getRewardFormatted() {
    const epu = this.getRewardElaPerUsd()
    const suffix = epu ? ` (@${epu} ELA/USD)` : ''
    return (
      this.getReward() && `${this.getReward()} ${this.getCurrency()}${suffix}`
    )
  }

  getBudgetExplanation() {
    return (
      <Popover content={I18N.get('task.budget.explain')}>
        <Icon className="help-icon" type="question-circle-o" />
      </Popover>
    )
  }

  getRewardExplanation() {
    return (
      <Popover content={I18N.get('task.reward.explain')}>
        <Icon className="help-icon" type="question-circle-o" />
      </Popover>
    )
  }

  getBudget() {
    if (!this.props.task.rewardUpfront) {
      return null
    }

    return this.props.task.rewardUpfront
      ? this.props.task.rewardUpfront.usd / 100
      : null
  }

  getBudgetElaPerUsd() {
    return (
      this.props.task.rewardUpfront && this.props.task.rewardUpfront.elaPerUsd
    )
  }

  getBudgetFormatted() {
    const epu = this.getBudgetElaPerUsd()
    const suffix = epu ? ` (@${epu} ELA/USD)` : ''
    return (
      this.getBudget() && `${this.getBudget()} ${this.getCurrency()}${suffix}`
    )
  }

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

  isTaskOwner() {
    return this.props.task.createdBy._id === this.props.currentUserId
  }

  linkProfileInfo(user) {
    this.setState({
      showUserInfo: user
    })
  }

  linkTeamInfo(userId) {
    this.props.history.push(`/team-detail/${userId}`)
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
      this.props.task.candidates,
      candidate =>
        candidate.user._id === userId &&
        candidate.status !== TASK_CANDIDATE_STATUS.REJECTED
    )
    if (!candidate) {
      return false
    }
    return this.withdrawApplication(candidate._id)
  }

  getImageCarousel() {
    const IMAGE_SIZE = 150

    const details = this.props.task
    const carouselImages = []

    if (details.thumbnail) {
      carouselImages.push(
        <img
          width={IMAGE_SIZE}
          height={IMAGE_SIZE}
          src={details.thumbnail}
          key="main"
        />
      )
    }

    for (const i of details.pictures) {
      carouselImages.push(
        <img width={IMAGE_SIZE} height={IMAGE_SIZE} src={i.url} key={i} />
      )
    }

    if (carouselImages.length === 0) {
      carouselImages.push(
        <img
          width={IMAGE_SIZE}
          height={IMAGE_SIZE}
          src="/assets/images/Group_1685.12.svg"
          key={0}
        />
      )
    }

    return (
      <div className="carousel-container">
        <div className="pictures-container">
          <Carousel autoplay={true}>{carouselImages}</Carousel>
        </div>
      </div>
    )
  }

  getCurrentContributorsData() {
    const detail = this.props.task
    return _.filter(detail.candidates, {
      status: TASK_CANDIDATE_STATUS.APPROVED
    })
  }

  getPendingCandidates() {
    const detail = this.props.task
    return _.filter(detail.candidates, {
      status: TASK_CANDIDATE_STATUS.PENDING
    })
  }

  canApply() {
    return (
      !this.hasAppliedBySelf() &&
      !_.some(this.props.ownedTeams, team => this.hasAppliedByTeam(team))
    )
  }

  showApplicationStartModal() {
    this.setState({
      showApplicationStartModal: true
    })
  }

  handleApplicationStartModalOk = e => {
    this.setState({
      showApplicationStartModal: false
    })
  }

  handleApplicationStartModalCancel() {
    this.setState({
      showApplicationStartModal: false
    })
  }
}

export default Form.create()(C)
