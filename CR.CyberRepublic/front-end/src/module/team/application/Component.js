import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import moment from 'moment'
import _ from 'lodash'
import {Col, Row, Button, Spin, Divider, message, List, Icon, Tooltip, Popconfirm, Card, Avatar} from 'antd'
import {TASK_CATEGORY, TASK_TYPE, TASK_STATUS, TASK_CANDIDATE_TYPE, TASK_CANDIDATE_STATUS} from '@/constant'
import Comments from '@/module/common/comments/Container'

const dateTimeFormat = 'MMM D, YYYY - h:mma (Z [GMT])'

export default class extends BaseComponent {
  ord_states() {
    return {}
  }

  ord_render() {
    return this.renderMain()
  }

  isTeamOwner() {
    return this.props.detail.createdBy._id === this.props.userId
  }

  getApplicant() {
    return _.find(this.props.detail.members, {_id: this.props.applicantId})
  }

  renderMain() {
    const applicant = this.getApplicant()
    const appliedDate = moment(applicant.createdAt).format('MMM D, YYYY')

    return (
      <div className="public">
        <Row>
          <Col span={24} className="gridCol main-area">
            <Row>
              <Col>
                <Card title={`Applied on ${appliedDate}`} extra={(
                  <div/>
                )}>
                  {applicant.user && (
                    <div>
                      <Avatar className="gap-right" src={applicant.user.profile.avatar}/>
                      {`${applicant.user.profile.firstName} ${applicant.user.profile.lastName}`}
                    </div>
                  )}
                  <h5>
                    {applicant.apply_reason}
                  </h5>
                </Card>
              </Col>
            </Row>
            <Row>
              <Comments type="teamCandidate" reduxType="team" canPost={true} model={applicant}
                detailReducer={(detail) => _.find(detail.members, {_id: this.props.applicantId})}
                returnUrl={`/team-detail/${this.props.detail._id}`}
              />
            </Row>
          </Col>
        </Row>
      </div>
    )
  }

  async withdrawApplication() {
    // const taskId = this.props.task._id
    // this.props.pullCandidate(taskId, tcId).then(() => {
    //     const prefix = this.props.page === 'LEADER'
    //         ? '/profile' : ''
    //     this.props.history.push(`${prefix}/task-detail/${this.props.task._id}`)
    // })
  }

  async rejectApplication() {

  }
}
