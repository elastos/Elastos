import React from 'react'
import AdminPage from '../BaseAdmin'
import moment from 'moment'
import _ from 'lodash'
import { logger } from '@/util'
import '../admin.scss'
import './style.scss'

import Navigator from '../../shared/HomeNavigator/Container'

import { Checkbox, Breadcrumb, Col, Icon, Row, Select, Input, Table, Popover, Popconfirm, message } from 'antd'
import { Link } from 'react-router-dom'

import config from '@/config'

export default class extends AdminPage {

  constructor(props) {
    super(props)

    this.state = {
      textFilter: (this.props.filter && this.props.filter.textFilter) || '',
      campaign: (this.props.filter && this.props.filter.campaign) || null,
      showArchived: (this.props.filter && this.props.filter.showArchived) ? this.props.filter.showArchived : false,
    }
  }

  async componentDidMount() {
    await super.componentDidMount()
    this.props.getSubmissions(this.state.showArchived)

  }

  componentWillUnmount() {
    this.props.resetSubmissions()
  }

  async handleSearch(value) {
    await this.setState({textFilter: value})

    this.saveFilter()
  }

  ord_renderContent () {

    let submissionData = this.props.all_submissions

    // filter results
    if (this.state.textFilter) {
      submissionData = submissionData.filter((submission) => {
        const regExp = new RegExp(this.state.textFilter, 'i')
        return (
          regExp.test(submission.title) ||
                    regExp.test(submission.description) ||
                    regExp.test(submission.fullLegalName)
        )
      })
    }

    if (this.state.campaign) {
      submissionData = submissionData.filter((submission) => {

        // test exact match
        if (submission.campaign === this.state.campaign) {
          return true
        }

        // empower 35
        if (submission.type === 'EMPOWER_35') {
          return true
        }

        return false
      })
    }

    const columns = [
      {
        title: 'Title',
        dataIndex: 'title',
        width: '20%',
        className: 'fontWeight500 allow-wrap',
        render: (name, record) => {
          return (
            <a onClick={this.linkSubmissionDetail.bind(this, record._id)} className="tableLink">
              {name}
              {record.archived &&
              <span className="no-info"> (archived)</span>
            }
            </a>
          )
        },
        sorter: (a, b) => {
          if (!a.title || !b.title) {
            return 0
          }
          return a.title.localeCompare(b.title)
        }

      }, {
        title: 'Name',
        dataIndex: 'fullLegalName',
        render: (fullLegalName, record) => {
          return fullLegalName || (record.createdBy ? (`${record.createdBy.profile.firstName} ${record.createdBy.profile.lastName}`) : '')
        }
      }, {
        title: 'Campaign',
        dataIndex: 'campaign',
        className: 'fontWeight500 allow-wrap',
        render: (campaign, record) => {
          return config.dict.formCampaigns[campaign] || _.capitalize(campaign)
        }
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
        width: '5%',
        render: (id, record) => {
          return (
            <div>
              <Popover content="archive">
                <Popconfirm title="Are you sure you want to archive this item?" placement="top" okText="Yes" onConfirm={this.archiveItem.bind(this, id)}>
                  <Icon type="inbox"/>
                </Popconfirm>
              </Popover>
            </div>
          )
        }
      }]

    return (
      <div className="p_admin_index ebp-wrap">
        <div className="ebp-header-divider" />
        <div className="d_box">
          <div className="p_admin_content">
            <Row>
              <Col span={4} className="wrap-box-navigator">
                <Navigator selectedItem="forms"/>
              </Col>
              <Col span={20} className="c_SubmissionTableContainer admin-right-column wrap-box-user">
                <div className="pull-right">
                  <Select
                    showSearch={true}
                    allowClear={true}
                    style={{width: 200, marginLeft: 8}}
                    placeholder="Select a campaign"
                    defaultValue={this.state.campaign || null}
                    onChange={this.selectCampaign.bind(this)}
                  >
                    {_.map(config.dict.formCampaigns, (campaign, key) => {
                      return (
                        <Select.Option key={key} value={key}>
                          {campaign}
                        </Select.Option>
                      )
                    })}
                  </Select>
                </div>
                <div className="pull-right">
                  <Input.Search onSearch={this.handleSearch.bind(this)}
                    defaultValue={this.state.textFilter || ''}
                    prefix={<Icon type="file-text" style={{color: 'rgba(0,0,0,.25)'}}/>}
                    placeholder="search"/>
                </div>
                <div className="showArchivedContainer pull-right">
                                    Show Archived
                                    &nbsp;
                  <Checkbox onClick={this.toggleShowArchived.bind(this)} checked={this.state.showArchived}/>
                </div>
                <div className="clearfix vert-gap-sm"/>
                <Table
                  columns={columns}
                  rowKey={(item) => item._id}
                  dataSource={submissionData}
                  loading={this.props.loading}
                />
              </Col>
            </Row>
          </div>
        </div>
      </div>
    )
  }

  // TODO: all UI should be moved from container to component
  async archiveItem(submissionId) {
    try {
      await this.props.archiveSubmission(submissionId, this.state.showArchived)
      message.success('Item archived successfully')
    } catch (err) {
      message.error('There was a problem archiving this item')
      logger.error(err)
    }
  }

  async toggleShowArchived() {

    await this.setState({
      showArchived: !this.state.showArchived
    })

    await this.props.showArchived(this.state.showArchived)

    this.saveFilter()
  }

  async selectCampaign(value) {
    await this.setState({
      campaign: value || ''
    })

    this.saveFilter()
  }

  saveFilter() {
    this.props.saveFilter(_.pick(this.state, ['textFilter', 'campaign', 'showArchived']))
  }

  linkSubmissionDetail(submissionId) {
    this.props.history.push(`/submission-detail/${submissionId}`)
  }
}
