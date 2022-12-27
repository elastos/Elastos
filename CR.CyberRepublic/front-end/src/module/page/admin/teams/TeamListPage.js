import React from 'react'
import BaseAdmin from '../BaseAdmin'
import {createContainer} from '@/util'

import Navigator from '../shared/Navigator/Component'
import { Breadcrumb, Col, Icon, Row, Table } from 'antd'

import TeamService from '@/service/TeamService'
import moment from 'moment/moment'
import config from '@/config'
import _ from 'lodash'
import {TEAM_TYPE} from '@/constant'
import MarkdownPreview from '@/module/common/MarkdownPreview'

const Component = class extends BaseAdmin {

  ord_renderContent() {
    return (
      <div className="p_admin_index ebp-wrap">
        <div className="ebp-header-divider" />
        <div className="d_box">
          <div className="p_admin_breadcrumb">
            <Breadcrumb>
              <Breadcrumb.Item href="/">
                <Icon type="home" />
              </Breadcrumb.Item>
              <Breadcrumb.Item>Admin</Breadcrumb.Item>
              <Breadcrumb.Item>teams</Breadcrumb.Item>
            </Breadcrumb>
          </div>
          <div className="p_admin_content">
            <Row>
              <Col span={4} className="admin-left-column wrap-box-navigator">
                <Navigator selectedItem="teams"/>
              </Col>
              <Col span={20} className="c_TaskTableContainer admin-right-column wrap-box-user">

                {this.renderList()}
              </Col>
            </Row>
          </div>
        </div>
      </div>
    )
  }

  renderList() {
    const columns = [
      {
        title: 'Name',
        dataIndex: 'name',
        key: 'name',
        width: '20%',
        className: 'fontWeight500',
        render: (name, record) => {
          return <a onClick={this.goDetail.bind(this, record._id)} className="tableLink">{name}</a>
        }
      },
      {
        title: 'Description',
        dataIndex: 'profile.description',
        render: (description, record) => {
          return <MarkdownPreview content={description} />
        }
        // key: 'profile.description'
      },
      {
        title: 'Type',
        dataIndex: 'type',
        key: 'type',
        // render: (category) => _.capitalize(category)
      },
      {
        title: 'Created',
        dataIndex: 'createdAt',
        key: 'createdAt',
        render: (createdAt) => moment(createdAt).format(config.FORMAT.DATE)
      }
    ]

    return (
      <Table
        columns={columns}
        rowKey={(item) => item._id}
        dataSource={this.props.all_teams}
        loading={this.props.loading}
      />
    )
  }

  goDetail(teamId) {
    this.props.history.push(`/admin/teams/${teamId}`)
  }

  async componentDidMount() {
    await super.componentDidMount()
    this.props.index({ type: TEAM_TYPE.TEAM })
  }
}

export default createContainer(Component, (state) => {
  const teamState = state.team

  if (!_.isArray(state.team.all_teams)) {
    teamState.all_teams = _.values(state.team.all_teams)
  }

  return teamState
}, () => {
  const teamService = new TeamService()

  return {
    async index(query) {
      await teamService.index(query)
    }
  }
})
