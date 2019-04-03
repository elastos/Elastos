import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import TaskCard from '@/module/common/TaskCard'
import _ from 'lodash'

import './style.scss'

import { Col, Row, Icon, Form, Input, Button, Spin } from 'antd'
import StandardPage from '../StandardPage'

const FormItem = Form.Item

export default class extends StandardPage {

  ord_states() {
    return {
      list: [],
      loading: false,
      total: 0
    }
  }

  ord_renderContent () {
    return (
      <div className="p_Tasks">
        <div className="ebp-header-divider" />
        <div className="ebp-page">
          {
            this.state.loading ? this.renderLoading() : this.renderList()
          }
        </div>
        <Footer />
      </div>
    )
  }

  renderLoading() {
    return (
      <div className="flex-center">
        <Spin size="large" />
      </div>

    )
  }

  renderList() {
    const list = _.chunk(this.state.list, 4)
    return (
      <div className="">
        {_.map(list, (d_list, i) => {
          return (
            <Row key={i} gutter={16} style={{marginBottom: 20}}>
              {
                _.map(d_list, (p, j) => {
                  return (
                    <Col key={j} className="gutter-row" span={4}>
                      <TaskCard {...p} />
                    </Col>
                  )
                })
              }
            </Row>
          )
        })}
      </div>
    )
  }

  componentDidMount() {
    this.setState({loading: true})
    this.props.fetchTaskList().then((d) => {

      this.setState({
        loading: false,
        list: d.list,
        total: d.total
      })
    })
  }
}
