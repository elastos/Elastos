import React from 'react'
import AdminPage from './BaseAdmin'
import moment from 'moment'
import _ from 'lodash'

import './admin.scss'

import Navigator from './shared/Navigator/Component'
import I18N from '@/I18N'
import { Breadcrumb, Col, Icon, Row, Input, Table, Select, Menu } from 'antd'

import {TASK_STATUS} from '@/constant'

export default class extends AdminPage {

  constructor(props) {
    super(props)

    // we use the props from the redux store if its retained
    this.state = {
      filteredInfo: null,
      pagination: (this.props.filter && this.props.filter.pagination) || null,
      taskNameFilter: (this.props.filter && this.props.filter.taskNameFilter) || '',
      statusFilter: (this.props.filter && this.props.filter.statusFilter) || null
    }
  }

  async componentDidMount() {
    await super.componentDidMount()
    this.props.getTasks()
  }

  componentWillUnmount() {
    this.props.resetTasks()
  }

  async handleChange(pagination, filters, sorter) {
    await this.setState({
      filteredInfo: filters,
      pagination
    })

    this.saveFilter()
  }

  async handleSearchTask(value) {
    await this.setState({taskNameFilter: value})

    this.saveFilter()
  }

  async changeStatusFilter(val, key) {
    await this.setState({
      statusFilter: (key && key.key) || null
    })

    this.saveFilter()
  }

  ord_getAllTasks() {
    return this.props.all_tasks
  }

  ord_getTaskPageName() {
    return ''
  }

  ord_renderContent () {
    let { filteredInfo } = this.state

    let taskData = this.ord_getAllTasks()

    if (this.state.taskNameFilter) {
      taskData = taskData.filter((task) => {
        const regExp = new RegExp(this.state.taskNameFilter, 'i')
        return (
          regExp.test(task.name) ||
                    (task.createdBy && regExp.test(task.createdBy.username))
        )
      })
    }

    if (this.state.statusFilter) {
      taskData = taskData.filter((task) => task.status === this.state.statusFilter)
    }

    filteredInfo = filteredInfo || {}

    const columns = [{
      title: 'Name',
      dataIndex: 'name',
      width: '30%',
      className: 'fontWeight500 allow-wrap',
      render: (name, record) => {
        return <a onClick={this.linkTaskDetail.bind(this, record._id)} className="tableLink">{name}</a>
      },
      sorter: (a, b) => a.name.localeCompare(b.name)
    }, {
      title: 'Owner',
      dataIndex: 'createdBy.username'
    }, {
      title: 'Category',
      dataIndex: 'category',
      render: (category) => (category === 'CR100' ? category : _.capitalize(category))
    }, {
      title: 'Type',
      dataIndex: 'type',
    }, {
      title: 'Community',
      dataIndex: 'community',
      key: 'community',
      render: (community, data) => {
        if (!community) {
          return null
        }

        if (data.communityParent) {
          const nameParent = data.communityParent.name
          return (
            <p>
              {nameParent}
/
              {community.name}
            </p>
          )
        }
        return (<p>{community.name}</p>)


      }
    }, {
      title: 'Status',
      dataIndex: 'status',
      /*
            filters: [
                {text: 'Created', value: TASK_STATUS.CREATED},
                {text: 'Pending', value: TASK_STATUS.PENDING},
                {text: 'Assigned', value: TASK_STATUS.ASSIGNED},
                {text: 'Approved', value: TASK_STATUS.APPROVED},
                {text: 'Submitted', value: TASK_STATUS.SUBMITTED},
                {text: 'Success', value: TASK_STATUS.SUCCESS},
                {text: 'Distributed', value: TASK_STATUS.DISTRIBUTED},
                {text: 'Canceled', value: TASK_STATUS.CANCELED},
                {text: 'Expired', value: TASK_STATUS.EXPIRED}
            ],
            filteredValue: filteredInfo.status || null,
            onFilter: (value, record) => record.status.includes(value),
            */
      className: 'fontWeight500',
      render: (val) => I18N.get(`taskStatus.${val}`)
    }, {
      title: 'Created',
      dataIndex: 'createdAt',
      render: (createdAt) => moment(createdAt).format('MMM D'),
      sorter: (a, b) => {
        return moment(a.createdAt).valueOf() - moment(b.createdAt).valueOf()
      },
      defaultSortOrder: 'descend'
    }, {
      title: '',
      dataIndex: '_id',
      key: 'actions',
      render: (id, record) => {

      }
    }]

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
              <Breadcrumb.Item>Tasks</Breadcrumb.Item>
            </Breadcrumb>
          </div>
          <div className="p_admin_content">
            <Row>
              <Col span={4} className="admin-left-column wrap-box-navigator">
                <Navigator selectedItem={this.ord_getTaskPageName()}/>
              </Col>
              <Col span={20} className="c_TaskTableContainer admin-right-column wrap-box-user">
                <div className="pull-right">
                  <Input.Search onSearch={this.handleSearchTask.bind(this)}
                    defaultValue={this.state.taskNameFilter || ''}
                    style={{width: 'auto'}}
                    prefix={<Icon type="solution" style={{color: 'rgba(0,0,0,.25)'}}/>}
                    placeholder="search task"/>

                  <Select
                    showSearch={true}
                    allowClear={true}
                    style={{width: 200, marginLeft: 8}}
                    placeholder="Select a status"
                    defaultValue={this.state.statusFilter}
                    onChange={this.changeStatusFilter.bind(this)}
                  >
                    {_.keys(TASK_STATUS).map((taskStatus) => {
                      return (
                        <Select.Option key={taskStatus} value={_.capitalize(taskStatus)}>
                          {I18N.get(`taskStatus.${taskStatus}`)}
                        </Select.Option>
                      )
                    })}
                  </Select>
                </div>
                <div className="clearfix vert-gap-sm"/>
                <Table
                  columns={columns}
                  rowKey={(item) => item._id}
                  dataSource={taskData}
                  loading={this.props.loading}
                  onChange={this.handleChange.bind(this)}
                  pagination={this.state.pagination}
                />
              </Col>
            </Row>
          </div>
        </div>
      </div>
    )
  }

  linkTaskDetail(taskId) {
    this.props.history.push(`/admin/task-detail/${taskId}`)
  }

  saveFilter() {
    this.props.saveFilter(_.pick(this.state, ['taskNameFilter', 'statusFilter', 'pagination']))
  }
}
