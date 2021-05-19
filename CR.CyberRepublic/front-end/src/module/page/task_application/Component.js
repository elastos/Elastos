import React from 'react'
import StandardPage from '../StandardPage'
import I18N from '@/I18N'
import Footer from '@/module/layout/Footer/Container'

import TaskApplicationDetail from '@/module/task/application/Container'

import _ from 'lodash'
import './style.scss'
import { Link } from 'react-router-dom'
import { Breadcrumb, Icon } from 'antd'

import {TASK_CATEGORY} from '@/constant'

export default class extends StandardPage {

  componentDidMount() {
    const taskId = this.props.match.params.taskId
    this.props.getTaskDetail(taskId)
  }

  componentWillUnmount() {
    this.props.resetTaskDetail()
  }

  ord_renderContent () {
    const candidate = (!_.isEmpty(this.props.task.candidates) &&
            this.props.task.candidates.find((candidate) => {
              return candidate.user._id === this.props.match.params.applicantId
            }))
    const detailLink = `/task-detail/${this.props.task._id}`

    return (
      <div className="p_TaskApplication">
        <div className="ebp-header-divider" />
        <div className="ebp-page">
          <TaskApplicationDetail task={this.props.task} applicantId={this.props.match.params.applicantId}/>
        </div>
        <Footer />
      </div>
    )
  }
}
