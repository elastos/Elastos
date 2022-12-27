import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import Navigator from '@/module/page/shared/HomeNavigator/Container'
import _ from 'lodash'

import './style.scss'
import '../../admin/admin.scss'

import { Col, Row, Icon, Select, Tooltip, Badge, Button, Table } from 'antd'
import moment from 'moment/moment'
import MediaQuery from 'react-responsive'
import { MAX_WIDTH_MOBILE, MIN_WIDTH_PC } from '@/config/constant'

import ProfilePage from '../../ProfilePage'
import I18N from '../../../../I18N/index';

const FILTERS = {
  ALL: 'all',
  CREATED: 'created',
  SUBSCRIBED: 'subscribed'
}

export default class extends ProfilePage {
  constructor(props) {
    super(props)

    this.state = {
      showMobile: false,
      filter: FILTERS.ALL
    }
  }

  componentDidMount() {
    super.componentDidMount()

    const query = {}

    if (!this.props.is_admin) {
      query.profileListFor = this.props.currentUserId
    }

    this.props.getSubmissions(query)
  }

  componentWillUnmount() {
    this.props.resetSubmissions()
  }

  getCommentActions(id, data) {
    const isOwner =
      data.createdBy && data.createdBy._id === this.props.currentUserId
    const subscription = _.find(data.subscribers, subscriber => {
      return subscriber.user && subscriber.user._id === this.props.currentUserId
    })
    const lastDate = isOwner
      ? data.lastCommentSeenByOwner
      : subscription && subscription.lastSeen

    const unread = _.filter(data.comments, comment => {
      return (
        !lastDate || new Date(_.first(comment).createdAt) > new Date(lastDate)
      )
    })
    const tooltipSuffix = unread.length > 1 ? 's' : ''
    const tooltip = `${unread.length} new message${tooltipSuffix}`

    return unread.length ? (
      <Tooltip title={tooltip}>
        <Badge dot={true} count={unread.length}>
          <a
            onClick={this.linkSubmissionDetail.bind(this, data._id)}
            className="tableLink"
          >
            <Icon type="message" />
          </a>
        </Badge>
      </Tooltip>
    ) : null
  }

  ord_renderContent() {
    const submissionsAllData = this.props.all_submissions
    const submissionsOwnedData = this.props.owned_submissions
    const submissionsSubscribedData = this.props.subscribed_submissions

    const columns = [
      {
        title: I18N.get('profile.submission.table.title'),
        dataIndex: 'title',
        width: '75%',
        className: 'fontWeight500 allow-wrap',
        render: (name, record) => {
          return (
            <a
              onClick={this.linkSubmissionDetail.bind(this, record._id)}
              className="tableLink"
            >
              {name}
            </a>
          )
        }
      },
      {
        title: I18N.get('profile.submission.table.type'),
        dataIndex: 'type',
        render: type => {
          if (type === 'FORM_EXT') {
            return 'FORM'
          }
          return type
        }
      },
      {
        title: I18N.get('profile.submission.table.created'),
        dataIndex: 'createdAt',
        className: 'right-align',
        render: createdAt => moment(createdAt).format('MMM D'),
        sorter: (a, b) => {
          return moment(a.createdAt).valueOf() - moment(b.createdAt).valueOf()
        },
        defaultSortOrder: 'descend'
      },
      {
        title: '',
        dataIndex: '_id',
        key: 'actions',
        render: this.getCommentActions.bind(this)
      }
    ]

    return (
      <div className="p_ProfileSubmissions">
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_admin_content">
              <Row>
                <Col sm={24} md={4} className="wrap-box-navigator">
                  <Navigator selectedItem="profileSubmissions" />
                </Col>
                <Col
                  sm={24}
                  md={20}
                  className="c_ProfileContainer admin-right-column wrap-box-user"
                >
                  <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
                    <Select
                      name="type"
                      onChange={this.onSelectFilter.bind(this)}
                      value={this.state.filter}
                    >
                      {_.map(FILTERS, (filter, key) => {
                        return (
                          <Select.Option key={filter} value={filter}>
                            {I18N.get(`profile.submission.filter.${filter}`)}
                          </Select.Option>
                        )
                      })}
                    </Select>
                  </MediaQuery>
                  <MediaQuery minWidth={MIN_WIDTH_PC}>
                    <Button.Group className="filter-group">
                      <Button
                        className={
                          (this.state.filter === FILTERS.ALL && 'selected') || ''
                        }
                        onClick={this.clearFilters.bind(this)}
                      >
                        {I18N.get('profile.submission.filter.all')}
                      </Button>
                      <Button
                        className={
                          (this.state.filter === FILTERS.CREATED && 'selected') || ''
                        }
                        onClick={this.setCreatedFilter.bind(this)}
                      >
                        {I18N.get('profile.submission.filter.created')}
                      </Button>
                      <Button
                        className={
                          (this.state.filter === FILTERS.SUBSCRIBED && 'selected') || ''
                        }
                        onClick={this.setSubscribedFilter.bind(this)}
                      >
                        {I18N.get('profile.submission.filter.subscribed')}
                      </Button>
                    </Button.Group>
                  </MediaQuery>
                  {this.state.filter === FILTERS.ALL && (
                    <div>
                      <Table
                        columns={columns}
                        rowKey={item => item._id}
                        dataSource={submissionsAllData}
                        loading={this.props.loading}
                      />
                    </div>
                  )}

                  {this.state.filter === FILTERS.CREATED && (
                    <div>
                      <Table
                        columns={columns}
                        rowKey={item => item._id}
                        dataSource={submissionsOwnedData}
                        loading={this.props.loading}
                      />
                    </div>
                  )}

                  {this.state.filter === FILTERS.SUBSCRIBED && (
                    <div>
                      <Table
                        columns={columns}
                        rowKey={item => item._id}
                        dataSource={submissionsSubscribedData}
                        loading={this.props.loading}
                      />
                    </div>
                  )}
                </Col>
              </Row>
              <Row>
                <Col>
                  <br />
                </Col>
              </Row>
            </div>
          </div>
        </div>
        <Footer />
      </div>
    )
  }

  onSelectFilter(value) {
    switch (value) {
      case FILTERS.CREATED:
        this.setCreatedFilter()
        break
      case FILTERS.SUBSCRIBED:
        this.setSubscribedFilter()
        break
      default:
        this.clearFilters()
        break
    }
  }

  clearFilters() {
    this.setState({ filter: FILTERS.ALL })
  }

  setCreatedFilter() {
    this.setState({ filter: FILTERS.CREATED })
  }

  setSubscribedFilter() {
    this.setState({ filter: FILTERS.SUBSCRIBED })
  }

  linkSubmissionDetail(submissionId) {
    this.props.history.push(`/profile/submission-detail/${submissionId}`)
  }

  goCreatepage() {
    this.props.history.push('/profile/submissions/create')
  }
}
