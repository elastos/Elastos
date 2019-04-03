import React from 'react'

import TaskApplicationDetail from '@/module/task/application/Container'
import Navigator from '@/module/page/shared/HomeNavigator/Container'
import _ from 'lodash'

import './style.scss'
import '../../admin/admin.scss'

import { Col, Row, Breadcrumb, Icon } from 'antd'

import ProfilePage from '../../ProfilePage'

export default class extends ProfilePage {

    state = {
      editing: false
    }

    componentDidMount() {
      const taskId = this.props.match.params.taskId
      this.props.getTaskDetail(taskId)
    }

    /*
    componentWillUnmount() {
        this.props.resetTaskDetail()
    }
    */

    getCandidate() {
      return (!_.isEmpty(this.props.task.candidates) &&
            this.props.task.candidates.find((candidate) => {
              return candidate.user._id === this.props.match.params.applicantId
            }))
    }

    ord_renderContent () {
      const candidate = this.getCandidate()
      const taskDetailLink = `/profile/task-detail/${this.props.task._id}`

      return (
        <div>
          <div className="ebp-header-divider" />
          <div className="p_admin_index ebp-wrap">
            <div className="d_box">
              <div className="p_ProfileTaskCandidateDetail p_admin_content">
                <Row>
                  <Col sm={24} md={4} className="admin-left-column wrap-box-navigator">
                    <Navigator selectedItem="profileTasks" />
                  </Col>
                  <Col sm={24} md={20} className="c_ProfileContainer admin-right-column wrap-box-user">
                    <TaskApplicationDetail task={this.props.task} page={this.props.page} applicantId={this.props.match.params.applicantId}/>
                  </Col>
                </Row>
              </div>
            </div>
          </div>
        </div>
      )
    }
}
