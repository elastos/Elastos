import React from 'react'
import StandardPage from '../StandardPage'
import Footer from '@/module/layout/Footer/Container'

// TODO: this is backwards and confusing
import TaskDetail from '@/module/task/Container'
import './style.scss'

export default class extends StandardPage {
  componentDidMount() {
    const taskId = this.props.match.params.taskId
    this.props.getTaskDetail(taskId)
  }

  componentWillUnmount() {
    this.props.resetTaskDetail()
  }

  ord_renderContent() {
    return (
      <div className="p_TaskDetail">
        <div className="ebp-header-divider" />
        <div className="ebp-page">
          <TaskDetail task={this.props.task} />
        </div>
        <Footer />
      </div>
    )
  }
}
