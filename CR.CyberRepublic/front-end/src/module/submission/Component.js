import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Col, Row, Icon, Divider, Button, Spin } from 'antd'
import _ from 'lodash'
import SubmissionDetail from './detail/Container'

import {TASK_STATUS} from '@/constant'

import './style.scss'

/**
 * This has 3 views
 *
 * 1. Public
 * 2. Admin
 * 3. Edit
 *
 */
export default class extends BaseComponent {

  renderMain() {
    return (
      <div className="c_SubmissionDetail">
        {this.renderDetail()}
      </div>
    )
  }

  renderDetail() {
    return <SubmissionDetail submission={this.props.submission} page={this.props.page}/>
  }

  ord_render () {
    return (_.isEmpty(this.props.submission) || this.props.submission.loading ?
      <div className="center"><Spin size="large" /></div> :
      this.renderMain()
    )
  }
}
