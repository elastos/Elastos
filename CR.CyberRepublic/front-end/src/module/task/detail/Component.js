import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import moment from 'moment'
import _ from 'lodash'
import {
  Col,
  Row,
  Button,
  Divider,
  message,
  Badge,
  List,
  Icon,
  Tooltip,
  Popconfirm,
  Spin,
  Popover
} from 'antd'

import ModalApplyTask from '../ModalApplyTask/Component'
import ModalAcceptApplicant from '../ModalAcceptApplicant/Component'

import {
  TASK_CATEGORY,
  TASK_TYPE,
  TASK_STATUS,
  TASK_CANDIDATE_TYPE,
  TASK_CANDIDATE_STATUS
} from '@/constant'
import Comments from '@/module/common/comments/Container'
import { TASK_EVENT_DATE_TYPE } from '../../../constant'

import I18N from '@/I18N'
import { getSafeUrl } from '@/util/url'
import { logger } from '@/util'

/*
 ************************************************************************************
 * DEPRECATED! - even tasks use the project/detail code now
 ************************************************************************************
 */

export default class extends BaseComponent {
  ord_states() {
    let acceptedCnt = 0

    if (this.props.task && _.isArray(this.props.task.candidates)) {
      for (const candidate of this.props.task.candidates) {
        if (candidate.status === TASK_CANDIDATE_STATUS.APPROVED) {
          acceptedCnt += 1
        }
      }
    }

    return {
      visibleModalApplyTask: false,
      visibleModalAcceptApplicant: false,
      visibleModalMemberProfile: false,
      acceptedCnt,
      selectedTaskCandidate: null,
      isDeveloperEvent:
        this.props.task.category === TASK_CATEGORY.DEVELOPER &&
        this.props.task.type === TASK_TYPE.EVENT,
      teamsOwned: []
    }
  }

  async componentDidMount() {
    this.setState({ loading: true })
    const teamsOwned = await this.props.listTeamsOwned(this.props.userId)

    this.setState({
      loading: false,
      teamsOwned: (teamsOwned && teamsOwned.list) || []
    })
  }

  componentWillUnmount() {
    // this may cause issues if a parent is checking the loading field
    this.props.resetTaskDetail()
  }

  ord_render() {
    const isTaskOwner =
      (this.props.task &&
        this.props.task.createdBy &&
        this.props.task.createdBy._id) === this.props.userId
    return (
      <div className="public">
        <Row>
          <Col sm={24} md={24} lg={18} className="gridCol main-area">
            <Row>
              <Col className="assignment-info">
                <h4 className="center">
                  {this.props.task.name}
                  {this.props.task.assignSelf && (
                    <span className="no-info"> - assigned to owner</span>
                  )}
                </h4>
              </Col>
            </Row>
            <Row>
              <Col span={this.props.task.thumbnail ? 18 : 24}>
                <Row>
                  <Col span={4} className="label-col">
                    Organizer
                  </Col>
                  <Col span={20}>
                    {this.props.task && this.props.task.createdBy ? (
                      <p>
                        <a
                          onClick={() => {
                            this.props.history.push(
                              `/member/${this.props.task.createdBy._id}`
                            )
                          }}
                        >
                          {this.props.task.createdBy.username}
                        </a>
                      </p>
                    ) : (
                      <div className="center">
                        <Spin size="small" />
                      </div>
                    )}
                  </Col>
                </Row>
                <Row>
                  <Col span={4} className="label-col">
                    Category
                  </Col>
                  <Col span={20}>
                    <p>
                      {I18N.get(
                        `developer.search.${this.props.task.category.toLowerCase()}`
                      )}
                    </p>
                  </Col>
                </Row>
                <Row>
                  <Col span={4} className="label-col">
                    Type
                  </Col>
                  <Col span={20}>
                    <p>{this.props.task.type}</p>
                  </Col>
                </Row>
                {this.props.task.community && (
                  <Row>
                    <Col span={4} className="label-col">
                      Community
                    </Col>
                    <Col span={20}>
                      <p>{this.getCommunityDisp()}</p>
                    </Col>
                  </Row>
                )}
                {this.props.task.applicationDeadline && (
                  <Row>
                    <Col span={4} className="label-col">
                      Application Deadline
                    </Col>
                    <Col span={20}>
                      {moment(this.props.task.applicationDeadline).format(
                        'MMM D, YYYY'
                      )}
                    </Col>
                  </Row>
                )}
                {this.props.task.completionDeadline && (
                  <Row>
                    <Col span={4} className="label-col">
                      Completion Deadline
                    </Col>
                    <Col span={20}>
                      {moment(this.props.task.completionDeadline).format(
                        'MMM D, YYYY'
                      )}
                    </Col>
                  </Row>
                )}
                <Row>
                  <Col span={4} className="label-col">
                    Description
                  </Col>
                  <Col span={20}>
                    <p>{this.props.task.description}</p>
                  </Col>
                </Row>
                {this.props.task.descBreakdown && (
                  <Row>
                    <Col sm={24} md={4}>
                      <span className="no-info">
                        Breakdown of Budget / Reward
                      </span>
                    </Col>
                    <Col sm={24} md={20}>
                      <p>{this.props.task.descBreakdown}</p>
                    </Col>
                  </Row>
                )}
                {this.props.task.goals && (
                  <Row>
                    <Col sm={24} md={4} className="label-col">
                      Goals
                    </Col>
                    <Col span={20}>
                      <p>{this.props.task.goals}</p>
                    </Col>
                  </Row>
                )}
                {this.props.task.infoLink && (
                  <Row>
                    <Col sm={24} md={4} className="label-col">
                      Info Link
                    </Col>
                    <Col sm={24} md={20}>
                      <a
                        target="_blank"
                        href={getSafeUrl(this.props.task.infoLink)}
                      >
                        {this.props.task.infoLink}
                      </a>
                    </Col>
                  </Row>
                )}

                {/*
                 ********************************************************************************
                 * Event Info
                 ********************************************************************************
                 */}
                {this.props.task.type === TASK_TYPE.EVENT && (
                  <div>
                    {this.props.task.eventDateRangeStart && (
                      <Row>
                        <Col span={4} className="label-col">
                          Date Start
                        </Col>
                        <Col span={8}>
                          {moment(this.props.task.eventDateRangeStart).format(
                            'MMM D, YYYY - HH:mm'
                          )}
                        </Col>
                        {this.props.task.eventDateRangeEnd && (
                          <Col span={4} className="label-col">
                            End
                          </Col>
                        )}
                        {this.props.task.eventDateRangeEnd && (
                          <Col span={8}>
                            {moment(this.props.task.eventDateRangeEnd).format(
                              'MMM D, YYYY - HH:mm'
                            )}
                          </Col>
                        )}
                      </Row>
                    )}
                    {this.props.task.eventDateRangeStart && (
                      <Row>
                        <Col span={4} className="label-col">
                          Confirmed
                        </Col>
                        <Col span={20}>
                          {(() => {
                            switch (this.props.task.eventDateStatus) {
                              case TASK_EVENT_DATE_TYPE.NOT_APPLICABLE:
                                return 'N/A'
                              case TASK_EVENT_DATE_TYPE.TENTATIVE:
                                return 'Tentative'
                              case TASK_EVENT_DATE_TYPE.CONFIRMED:
                                return 'Confirmed'
                            }
                          })()}
                        </Col>
                      </Row>
                    )}
                    <Row>
                      <Col span={4} className="label-col">
                        Location
                      </Col>
                      <Col span={20}>{this.props.task.location}</Col>
                    </Row>
                  </div>
                )}
                {this.props.task.reward &&
                (this.props.task.reward.usd ||
                  this.props.task.rewardUpfront.usd ||
                  this.props.task.reward.ela ||
                  this.props.task.rewardUpfront.ela) ? (
                  this.props.task.reward && this.props.task.reward.isUsd ? (
                    <div>
                      <Divider>
                        Budget / Reward&nbsp;
                        <Popover content="Budget is for expenses/costs, reward is for labor and time">
                          <Icon
                            className="help-icon"
                            type="question-circle-o"
                          />
                        </Popover>
                      </Divider>
                      <Row>
                        <Col span={4} className="label-col">
                          USD Budget
                        </Col>
                        <Col span={8}>
                          <p>{this.props.task.rewardUpfront.usd / 100}</p>
                        </Col>
                        {this.props.task.rewardUpfront.elaPerUsd > 0 && (
                          <Col span={4} className="label-col">
                            ELA/USD
                          </Col>
                        )}
                        {this.props.task.rewardUpfront.elaPerUsd > 0 && (
                          <Col span={8}>
                            <p>{this.props.task.rewardUpfront.elaPerUsd}</p>
                          </Col>
                        )}
                      </Row>
                      <Row>
                        <Col span={4} className="label-col">
                          USD Reward
                        </Col>
                        <Col span={8}>
                          <p>{this.props.task.reward.usd / 100}</p>
                        </Col>
                        {this.props.task.reward.elaPerUsd > 0 && (
                          <Col span={4} className="label-col">
                            ELA/USD
                          </Col>
                        )}
                        {this.props.task.reward.elaPerUsd > 0 && (
                          <Col span={8}>
                            <p>{this.props.task.reward.elaPerUsd}</p>
                          </Col>
                        )}
                      </Row>
                    </div>
                  ) : (
                    <div>
                      <Row>
                        <Col sm={24} md={4} className="label-col">
                          ELA Budget
                        </Col>
                        <Col sm={24} md={20}>
                          <p>
                            {this.props.task.rewardUpfront &&
                              this.props.task.rewardUpfront.ela / 1000}
                          </p>
                        </Col>
                      </Row>
                      <Row>
                        <Col sm={24} md={4} className="label-col">
                          ELA Reward
                        </Col>
                        <Col sm={24} md={20}>
                          <p>
                            {this.props.task.reward &&
                              this.props.task.reward.ela / 1000}
                          </p>
                        </Col>
                      </Row>
                    </div>
                  )
                ) : (
                  <div />
                )}
                {this.props.task.approvedBy && (
                  <Row>
                    <Col sm={24} md={4}>
                      <span className="no-info">Approved By</span>
                    </Col>
                    <Col sm={24} md={20}>
                      <p>
                        {this.props.task.approvedBy.username}
                        {this.props.task.approvedDate && (
                          <span>
                            &nbsp; on{' '}
                            {moment(this.props.task.approvedDate).format(
                              'MMM D, YYYY'
                            )}
                          </span>
                        )}
                      </p>
                    </Col>
                  </Row>
                )}
              </Col>
              {this.props.task.thumbnail && (
                <Col span={6}>
                  <img
                    src={this.props.task.thumbnail}
                    className="task-thumbnail"
                  />
                </Col>
              )}
            </Row>

            {/*
             ********************************************************************************
             * Attachment
             ********************************************************************************
             */}
            {this.props.task.attachment && (
              <div>
                <div className="vert-gap" />
                <Divider>Attachment</Divider>

                <Row>
                  <Col span={4} className="label-col">
                    File
                  </Col>
                  <Col span={20}>
                    <a
                      target="_blank"
                      href={getSafeUrl(this.props.task.attachment)}
                    >
                      {this.props.task.attachmentType === 'application/pdf' ? (
                        <Icon type="file-pdf" />
                      ) : (
                        <Icon type="file" />
                      )}{' '}
                      &nbsp;
                      {this.props.task.attachmentFilename}
                    </a>
                  </Col>
                </Row>
              </div>
            )}

            <Comments
              type="task"
              canPost={true}
              canSubscribe={!isTaskOwner}
              model={this.props.task}
              returnUrl={`/task-detail/${this.props.task._id}`}
            />
          </Col>
          {/*
           ********************************************************************************
           * Applicants (Right Col)
           ********************************************************************************
           */}
          <Col sm={24} lg={6} className="gridCol applicants">
            <h4>
              {this.state.isDeveloperEvent ? 'Registrants' : 'Applicants'}
            </h4>

            {this.props.task.candidates && this.props.task.candidates.length ? (
              <List
                size="small"
                dataSource={this.props.task.candidates}
                renderItem={candidate => {
                  const name =
                    candidate.type === TASK_CANDIDATE_TYPE.USER
                      ? candidate.user && candidate.user.username
                      : candidate.team && candidate.team.name
                  const lastSeen = isTaskOwner
                    ? candidate.lastSeenByOwner
                    : candidate.lastSeenByCandidate
                  const unread = _.filter(candidate.comments, comment => {
                    return (
                      !lastSeen ||
                      new Date(_.first(comment).createdAt) > new Date(lastSeen)
                    )
                  })
                  const listItemActions = []

                  if (
                    this.state.isDeveloperEvent &&
                    candidate.type === TASK_CANDIDATE_TYPE.TEAM
                  ) {
                    listItemActions.unshift(
                      <Tooltip title="Team">
                        <Icon type="team" />
                      </Tooltip>
                    )
                  }

                  const candidateIsUserOrTeam =
                    (candidate.type === TASK_CANDIDATE_TYPE.USER &&
                      candidate.user._id === this.props.userId) ||
                    (candidate.type === TASK_CANDIDATE_TYPE.TEAM &&
                      _.map(this.state.teamsOwned, '_id').includes(
                        candidate.team._id
                      ))

                  const isLeader =
                    this.props.page === 'LEADER' &&
                    isTaskOwner &&
                    !candidateIsUserOrTeam
                  // we either show the remove icon or the approved icon,
                  // after approval the user cannot rescind their application

                  // if the candidate is the logged in user, show remove icon
                  if (candidateIsUserOrTeam) {
                    if (this.state.isDeveloperEvent) {
                      listItemActions.unshift(
                        <Tooltip title="Withdraw application">
                          <a
                            onClick={this.removeApplication.bind(
                              this,
                              candidate._id
                            )}
                          >
                            <Icon type="close-circle-o" />
                          </a>
                        </Tooltip>
                      )
                    } else {
                      // non developer events should confirm
                      if (candidate.type === TASK_CANDIDATE_TYPE.USER) {
                        listItemActions.unshift(
                          <Tooltip title="Withdraw application">
                            <Popconfirm
                              title="Are you sure you want to withdraw your application?"
                              onConfirm={this.removeApplication.bind(
                                this,
                                candidate._id
                              )}
                              placement="left"
                              okText="Yes"
                              cancelText="No"
                            >
                              <Icon type="close-circle-o" />
                            </Popconfirm>
                          </Tooltip>
                        )
                      } else if (candidate.type === TASK_CANDIDATE_TYPE.TEAM) {
                        listItemActions.unshift(
                          <Tooltip title="remove team">
                            <Popconfirm
                              title="Are you sure you want to withdraw your team's application?"
                              onConfirm={this.removeApplication.bind(
                                this,
                                candidate._id
                              )}
                              placement="left"
                              okText="Yes"
                              cancelText="No"
                            >
                              <Icon type="close-circle-o" />
                            </Popconfirm>
                          </Tooltip>
                        )
                      }
                    }
                  } else if (
                    candidate.status === TASK_CANDIDATE_STATUS.APPROVED
                  ) {
                    // this should be the leader's view - they can approve applicants
                    listItemActions.unshift(
                      <Tooltip
                        title={
                          isTaskOwner
                            ? candidateIsUserOrTeam
                              ? 'You are automatically accepted'
                              : 'Candidate already accepted'
                            : 'Accepted candidate'
                        }
                      >
                        <Icon type="check-circle" />
                      </Tooltip>
                    )
                  }

                  if (isTaskOwner) {
                    if (candidate.status !== TASK_CANDIDATE_STATUS.APPROVED) {
                      listItemActions.unshift(
                        <Tooltip title="Accept application">
                          <a
                            onClick={this.showModalAcceptApplicant.bind(
                              this,
                              candidate
                            )}
                          >
                            <Icon type="check-circle-o" />
                          </a>
                        </Tooltip>
                      )
                    }

                    const prefix =
                      this.props.page === 'LEADER' ? '/profile' : ''
                    listItemActions.unshift(
                      <Tooltip title="View application">
                        <a
                          onClick={() => {
                            this.props.history.push(
                              `${prefix}/task-app/${this.props.task._id}/${candidate.user._id}`
                            )
                          }}
                        >
                          <Icon type="info-circle-o" />
                        </a>
                      </Tooltip>
                    )
                  }

                  if (unread.length) {
                    const suffix = unread.length > 1 ? 's' : ''
                    const title = `${unread.length} new message${suffix}`
                    listItemActions.unshift(
                      <Tooltip title={title}>
                        <Badge dot={true} count={unread.length}>
                          <Icon type="message" />
                        </Badge>
                      </Tooltip>
                    )
                  }

                  const userOrTeamName = name
                  const selfIcon = candidateIsUserOrTeam ? (
                    <Icon type="user" />
                  ) : null

                  return (
                    <List.Item actions={listItemActions}>
                      <Tooltip title="View profile">
                        <a
                          onClick={() => {
                            this.props.history.push(
                              `/member/${candidate.user._id}`
                            )
                          }}
                        >
                          {selfIcon}
                          {userOrTeamName}
                        </a>
                      </Tooltip>
                    </List.Item>
                  )
                }}
              />
            ) : (
              <span className="no-info">
                {this.props.task.status === TASK_STATUS.PENDING
                  ? 'task must be approved first'
                  : this.state.isDeveloperEvent
                  ? 'no registrants'
                  : 'no applicants'}
              </span>
            )}

            {this.props.is_login && !isTaskOwner && this.renderJoinButton()}
          </Col>
        </Row>

        <ModalApplyTask
          wrappedComponentRef={this.saveFormApplyTaskRef}
          teamsOwned={this.state.teamsOwned}
          visible={this.state.visibleModalApplyTask}
          onCancel={this.handleCancelModalApplyTask}
          onCreate={this.handleModalApplyTask}
        />

        <ModalAcceptApplicant
          wrappedComponentRef={this.saveAcceptCandidateRef}
          acceptedCnt={this.state.acceptedCnt}
          acceptedMax={this.props.task.candidateSltLimit}
          taskCandidate={this.state.modalTaskCandidate}
          visible={this.state.visibleModalAcceptApplicant}
          onCancel={this.handleCancelModalAcceptApplicant}
          onCreate={this.handleModalAcceptApplicant}
        />
      </div>
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

  /**
   * Developer Events - multiple people can simply join
   * Developer Tasks - members must apply for task, with a certain # to be selected
   *
   * Social Events - members must apply to help
   * Social Tasks - members must apply for task, with a certain # to be selected
   */
  renderJoinButton() {
    if (this.state.isDeveloperEvent) {
      return (
        <Button
          disabled={true}
          className="join-btn"
          onClick={this.confirmDeveloperEventJoin}
        >
          Join Event
        </Button>
      )
      // shortcuts here
    }

    let buttonText = ''
    const appliedAlready = _.find(this.props.task.candidates, candidate => {
      return (
        candidate && candidate.user && candidate.user._id === this.props.userId
      )
    })

    if (!appliedAlready) {
      if (this.props.task.type === TASK_TYPE.TASK) {
        buttonText = 'Apply for Task'
      } else {
        buttonText = 'Apply to Help'
      }

      return (
        <Button
          disabled={true}
          className="join-btn"
          onClick={this.showModalApplyTask}
        >
          {buttonText}
        </Button>
      )
    }

    // else
    buttonText = 'My Application'
    const prefix = this.props.page === 'LEADER' ? '/profile' : ''

    return (
      <Button
        disabled={true}
        className="join-btn"
        onClick={() => {
          this.props.history.push(
            `${prefix}/task-app/${this.props.task._id}/${this.props.userId}`
          )
        }}
      >
        {buttonText}
      </Button>
    )
  }

  /**
   * First we show a modal with teams or apply alone, only
   * the leader of a team can apply
   */
  showModalApplyTask = () => {
    this.formRefApplyTask.props.form.setFieldsValue({}, () => {
      this.setState({
        visibleModalApplyTask: true
      })
    })
  }

  saveFormApplyTaskRef = formRef => {
    this.formRefApplyTask = formRef
  }

  handleCancelModalApplyTask = () => {
    const form = this.formRefApplyTask.props.form
    form.resetFields()

    this.setState({ visibleModalApplyTask: false })
  }

  handleModalApplyTask = () => {
    const form = this.formRefApplyTask.props.form

    // applyId is either literally 'self' or a teamId
    const applyId = form.getFieldValue('applyId')

    const isSelf = form.getFieldValue('applyId') === 'self'

    let userId, teamId

    if (isSelf) {
      // we push our own id
      userId = this.props.userId
    } else {
      teamId = applyId
    }

    const taskId = this.props.task._id
    const applyMsg = form.getFieldValue('applyMsg') || ''

    this.handleCancelModalApplyTask()

    // TODO: throwing an error in pushCandidate doesn't seem to trigger the catch error block
    this.props
      .pushCandidate(taskId, userId, teamId, applyMsg)
      .then(result => {
        if (result) {
          message.success('You have applied, you will be contacted if approved')
        }
      })
      .catch(err => {
        // never entered
        message.error(err.message, 10)
        logger.error(err)
      })
  }

  async removeApplication(tcId) {
    const taskId = this.props.task._id
    const res = await this.props.pullCandidate(taskId, tcId)
  }

  /**
   * For organizers they can accept an applicant
   */
  showModalAcceptApplicant = taskCandidate => {
    this.setState({
      modalTaskCandidate: taskCandidate,
      visibleModalAcceptApplicant: true
    })
  }

  saveAcceptCandidateRef = ref => {
    this.acceptCandidateRef = ref
  }

  handleCancelModalAcceptApplicant = () => {
    this.setState({ visibleModalAcceptApplicant: false })
  }

  handleModalAcceptApplicant = () => {
    // this is the candidate we are accepting
    const taskCandidateId = this.state.modalTaskCandidate._id
    this.handleCancelModalAcceptApplicant()

    this.props
      .acceptCandidate(taskCandidateId)
      .then(result => {
        message.success('Applicant has been accepted and contacted')

        let acceptedCnt = this.state.acceptedCnt

        acceptedCnt += 1

        this.setState({ acceptedCnt })
      })
      .catch(err => {
        message.error(err.message, 10)
        logger.error(err)
      })
  }

  /**
   * If it's a developer event, we simply accept the registration
   *
   * // TODO: send the reminder
   */
  confirmDeveloperEventJoin = () => {
    message.success(
      'You have successfully registered for this event, a reminder will be sent closer to the event date'
    )

    const taskId = this.props.task._id
    const userId = this.props.userId

    this.props.pushCandidate(taskId, userId)
  }
  // TODO: after max applicants are selected, we should send an email to those
  // that were not selected
}
