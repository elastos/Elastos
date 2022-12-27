import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import ModalAcceptApplicant from '../ModalAcceptApplicant/Component'
import { logger } from '@/util'
import _ from 'lodash'

import { Col, Row, Button, Spin, Divider, message, List, Icon, Tooltip, Popconfirm } from 'antd'

import {TASK_CATEGORY, TASK_TYPE, TASK_STATUS, TASK_CANDIDATE_TYPE, TASK_CANDIDATE_STATUS} from '@/constant'
import Comments from '@/module/common/comments/Container'


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
      visibleModalAcceptApplicant: false,
      acceptedCnt,
      selectedTaskCandidate: null,
      isDeveloperEvent: this.props.task.category === TASK_CATEGORY.DEVELOPER &&
                                this.props.task.type === TASK_TYPE.EVENT,
      teamsOwned: []
    }
  }

  async componentDidMount() {
    this.setState({loading: true})
    const teamsOwned = await this.props.listTeamsOwned(this.props.userId)

    this.setState({
      loading: false,
      teamsOwned: (teamsOwned && teamsOwned.list) || []
    })

    const taskId = this.props.match.params.taskId
    taskId && this.props.getTaskDetail(taskId).then(() => {
      const candidate = this.getApplicant()
      candidate && this.props.markVisited(candidate._id, this.isTaskOwner())
    })
  }

  ord_render () {
    return (_.isEmpty(this.props.task) || this.props.task.loading ?
      <div className="center"><Spin size="large" /></div> :
      this.renderMain()
    )
  }

  isTaskOwner() {
    return this.props.task.createdBy._id === this.props.userId
  }

  getBanner() {
    const isTaskOwner = this.isTaskOwner()
    const applicant = this.getApplicant()

    let bannerInsides = null
    if (applicant.complete) {
      bannerInsides = (
        <span className="help-text">Task marked as complete</span>
      )
    } else if (isTaskOwner && !applicant.complete) {
      bannerInsides = (
        <span className="help-text">Task incomplete</span>
      )
    } else if (!applicant.complete) {
      bannerInsides = (
        <div>
          <span className="help-text">Task incomplete</span>
          <div className="pull-right right-align">
            <Popconfirm title="Are you sure you want to mark this complete?" placement="left" okText="Yes" onConfirm={this.markComplete.bind(this)}>
              <Button type="primary">Mark Complete</Button>
            </Popconfirm>
          </div>
          <div className="clearfix"/>
        </div>
      )
    }

    return bannerInsides
  }

  getApplicant () {
    return (!_.isEmpty(this.props.task.candidates) &&
            this.props.task.candidates.find((candidate) => {
              return candidate.user._id === this.props.applicantId
            }))
  }

  markComplete () {
    const applicant = this.getApplicant()
    this.props.markComplete(applicant._id)
  }

  renderMain () {
    const isTaskOwner = this.props.task.createdBy._id === this.props.userId
    const applicant = this.getApplicant()
    const banner = this.getBanner()

    return (
      <div className="public">
        <Row>
          <Col span={24} className="gridCol banner-area">
            <div className="l_banner">
              {banner}
            </div>
          </Col>
          <Col span={18} className="gridCol main-area">
            <Row>
              <Col>
                <h4 className="center">
                  {applicant.applyMsg}
                </h4>
              </Col>
            </Row>
            <Comments type="taskCandidate" reduxType="task" canPost={true} model={applicant}
              detailReducer={(detail) => _.find(detail.candidates, (candidate) => {
                return candidate.user._id === this.props.match.params.applicantId
              })}
              returnUrl={`/task-detail/${this.props.task._id}`}
            />
          </Col>
          <Col span={6} className="gridCol applicants">
            <h4>{this.state.isDeveloperEvent ? 'Registrants' : 'Applicants'}</h4>

            {(this.props.task.candidates && this.props.task.candidates.length) ? (
              <List
                size="small"
                dataSource={this.props.task.candidates}
                renderItem={(candidate) => {

                  const name = candidate.type === TASK_CANDIDATE_TYPE.USER ? candidate.user.username : candidate.team.name
                  const listItemActions = []

                  if (this.state.isDeveloperEvent) {
                    listItemActions.push(candidate.type === TASK_CANDIDATE_TYPE.USER ? (
                      <Tooltip title="Solo User">
                        <Icon type="user"/>
                      </Tooltip>
                    ) : (
                      <Tooltip title="Team">
                        <Icon type="team"/>
                      </Tooltip>
                    ))
                  }

                  let candidateIsUserOrTeam = false
                  if ((candidate.type === TASK_CANDIDATE_TYPE.USER && candidate.user._id === this.props.userId) ||
                                    (candidate.type === TASK_CANDIDATE_TYPE.TEAM && _.map(this.state.teamsOwned, '_id').includes(candidate.team._id))) {

                    candidateIsUserOrTeam = true
                  }

                  const isLeader = this.props.page === 'LEADER' && isTaskOwner && !candidateIsUserOrTeam

                  // we either show the remove icon or the approved icon,
                  // after approval the user cannot rescind their application

                  // if the candidate is the logged in user, show remove icon
                  if (candidateIsUserOrTeam) {
                    if (this.state.isDeveloperEvent) {
                      listItemActions.unshift(
                        <Tooltip title="Withdraw application">
                          <a onClick={this.removeApplication.bind(this, candidate._id)}>
                            <Icon type="close-circle-o"/>
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
                              onConfirm={this.removeApplication.bind(this, candidate._id)}
                              placement="left"
                              okText="Yes"
                              cancelText="No"
                            >
                              <Icon type="close-circle-o"/>
                            </Popconfirm>
                          </Tooltip>
                        )
                      } else if (candidate.type === TASK_CANDIDATE_TYPE.TEAM) {
                        listItemActions.unshift(
                          <Tooltip title="remove team">
                            <Popconfirm
                              title="Are you sure you want to remove your team's application?"
                              onConfirm={this.removeApplication.bind(this, candidate._id)}
                              placement="left"
                              okText="Yes"
                              cancelText="No"
                            >
                              <Icon type="close-circle-o"/>
                            </Popconfirm>
                          </Tooltip>
                        )
                      }
                    }
                  } else if (candidate.status === TASK_CANDIDATE_STATUS.APPROVED) {
                    // this should be the leader's view - they can approve applicants
                    listItemActions.unshift(
                      <Tooltip title={isTaskOwner ? (candidateIsUserOrTeam ? 'you are automatically accepted' : 'candidate already accepted') : 'accepted candidate'}>
                        <a>✓</a>
                      </Tooltip>
                    )
                  }

                  if (isTaskOwner) {
                    listItemActions.unshift(
                      <Tooltip title="Accept application">
                        <a onClick={this.showModalAcceptApplicant.bind(this, candidate)}>
                          <Icon type="check-circle-o" />
                        </a>
                      </Tooltip>
                    )
                    const prefix = this.props.page === 'LEADER'
                      ? '/profile'
                      : ''
                    listItemActions.unshift(
                      <Tooltip title="View application">
                        <a onClick={() => {this.props.history.push(`${prefix}/task-app/${this.props.task._id}/${candidate.user._id}`)}}>
                          <Icon type="info-circle-o"/>
                        </a>
                      </Tooltip>
                    )
                  }

                  const userOrTeamName = name
                  const selfIcon = candidateIsUserOrTeam
                    ?
                    (
                      <Icon type="user"/>
                    )
                    : null

                  const isCurrent = candidate.id === applicant.id
                  const currentClass = isCurrent ? 'active' : ''
                  return (
                    <List.Item actions={listItemActions} className={currentClass}>
                      <Tooltip title="View profile">
                        <a onClick={() => {this.props.history.push(`/member/${candidate.user._id}`)}}>
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
                {this.props.task.status === TASK_STATUS.PENDING ? 'task must be approved first' : (this.state.isDeveloperEvent ? 'no registrants' : 'no applicants')}
              </span>
            )
            }

          </Col>
        </Row>
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

  async removeApplication(tcId) {
    const taskId = this.props.task._id
    this.props.pullCandidate(taskId, tcId).then(() => {
      const prefix = this.props.page === 'LEADER'
        ? '/profile' : ''
      this.props.history.push(`${prefix}/task-detail/${this.props.task._id}`)
    })
  }

    showModalAcceptApplicant = (taskCandidate) => {
      this.setState({
        modalTaskCandidate: taskCandidate,
        visibleModalAcceptApplicant: true
      })
    }

    saveAcceptCandidateRef = (ref) => {
      this.acceptCandidateRef = ref
    }

    handleCancelModalAcceptApplicant = () => {
      this.setState({visibleModalAcceptApplicant: false})
    }

    handleModalAcceptApplicant = () => {
      // this is the candidate we are accepting
      const taskCandidateId = this.state.modalTaskCandidate._id
      this.handleCancelModalAcceptApplicant()

      this.props.acceptCandidate(taskCandidateId).then((result) => {
        message.success('Applicant has been accepted and contacted')

        let acceptedCnt = this.state.acceptedCnt

        acceptedCnt += 1

        this.setState({acceptedCnt})

      }).catch((err) => {
        message.error(err.message, 10)
        logger.error(err)
      })
    }
}
