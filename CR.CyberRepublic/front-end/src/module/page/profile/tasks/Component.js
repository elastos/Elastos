import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import Navigator from '@/module/page/shared/HomeNavigator/Container'
import _ from 'lodash'
import I18N from '@/I18N'
import MarkdownPreview from '@/module/common/MarkdownPreview'

import './style.scss'
import '../../admin/admin.scss'

import {
  TASK_CANDIDATE_STATUS,
  USER_AVATAR_DEFAULT,
  TASK_STATUS
} from '@/constant'
import {
  Col,
  Row,
  Icon,
  Select,
  Badge,
  Tooltip,
  Avatar,
  Button,
  Tag,
  Divider,
  List,
  Carousel,
  Input
} from 'antd'

import moment from 'moment/moment'
import MediaQuery from 'react-responsive'
import ProfilePage from '../../ProfilePage'
import { MAX_WIDTH_MOBILE, MIN_WIDTH_PC } from '../../../../config/constant'

const FILTERS = {
  ALL: 'all',
  ACTIVE: 'active',
  APPLIED: 'applied',
  OWNED: 'owned',
  SUBSCRIBED: 'subscribed',
  NEED_APPROVAL: 'need_approval'
}

/**
 * This uses new features such as infinite scroll and pagination, therefore
 * we do some different things such as only loading the data from the server
 */
export default class extends ProfilePage {
  constructor(props) {
    super(props)

    // we use the props from the redux store if its retained
    this.state = {
      showMobile: false,
      filter: FILTERS.ALL,
      statusFilter: this.props.filter && this.props.filter.statusFilter,
      page: 1,
      results: 5,
      search: ''
    }

    this.debouncedLoadMore = _.debounce(this.loadMore.bind(this), 300)
    this.debouncedRefetch = _.debounce(this.refetch.bind(this), 300)
  }

  componentDidMount() {
    super.componentDidMount()
    this.refetch()
    this.props.getUserTeams(this.props.currentUserId)
  }

  componentWillUnmount() {
    this.props.resetTasks()
  }

  /**
   * Builds the query from the current state
   */
  getQuery() {
    const query = {
      profileListFor: this.props.currentUserId
    }

    if (this.state.filter === FILTERS.ACTIVE) {
      query.taskHasUserStatus = TASK_CANDIDATE_STATUS.APPROVED
    }

    if (this.state.filter === FILTERS.APPLIED) {
      query.taskHasUserStatus = TASK_CANDIDATE_STATUS.PENDING
    }

    if (this.state.filter === FILTERS.OWNED) {
      query.createdBy = this.props.currentUserId
    }

    if (this.state.filter === FILTERS.SUBSCRIBED) {
      query.subscribed = true
    }

    if (this.state.filter === FILTERS.NEED_APPROVAL) {
      query.status = TASK_STATUS.PENDING
    }

    if (!_.isEmpty(this.state.search)) {
      query.search = this.state.search
    }

    // let this override, though it should be mutually exclusive with state.filter
    if (this.state.statusFilter) {
      query.status = this.state.statusFilter.toUpperCase()
    }

    query.page = this.state.page || 1
    query.results = this.state.results || 5

    return query
  }

  /**
   * Refetch the data based on the current state retrieved from getQuery
   */
  refetch() {
    const query = this.getQuery()
    this.props.getTasks(query)
  }

  async loadMore() {
    const page = this.state.page + 1

    const query = {
      ...this.getQuery(),
      page,
      results: this.state.results
    }

    this.setState({ loadingMore: true })

    try {
      await this.props.loadMoreTasks(query)
      this.setState({ page })
    } catch (e) {
      // Do not update page in state if the call fails
    }

    this.setState({ loadingMore: false })
  }

  async loadPage(page) {
    const query = {
      ...this.getQuery(),
      page,
      results: this.state.results
    }

    this.setState({ loadingMore: true })

    try {
      await this.props.loadMoreTasks(query)
      this.setState({ page })
    } catch (e) {
      // Do not update page in state if the call fails
    }

    this.setState({ loadingMore: false })
  }

  hasMoreTasks() {
    return _.size(this.props.all_tasks) < this.props.all_tasks_total
  }

  getOwnerCommentActions(id, data) {
    const candidateActions = this.getCandidateCommentActions(
      'lastSeenByOwner',
      id,
      data
    )
    const commentActions = this.getCommentActions(id, data)

    return (
      <div>
        {candidateActions}
        {commentActions}
      </div>
    )
  }

  getCandidateCommentActions(seenProperty, id, data) {
    const candidate = _.find(data.candidates, candidate => {
      return candidate.user && candidate.user._id === this.props.currentUserId
    })
    let unread = []

    if (candidate) {
      const lastDate = candidate[seenProperty]
      unread = _.filter(candidate.comments, comment => {
        return (
          !lastDate || new Date(_.first(comment).createdAt) > new Date(lastDate)
        )
      })
    } else {
      unread = _.flatten(
        _.map(data.candidates, candidate => {
          const lastDate = candidate[seenProperty]
          const subUnread = _.filter(candidate.comments, comment => {
            return (
              !lastDate ||
              new Date(_.first(comment).createdAt) > new Date(lastDate)
            )
          })
          return subUnread
        })
      )
    }

    const tooltipSuffix = unread.length > 1 ? 's' : ''
    const tooltip = `${unread.length} new message${tooltipSuffix}`

    return unread.length ? (
      <Tooltip title={tooltip}>
        <Badge dot={true} count={unread.length}>
          <a
            onClick={
              candidate
                ? this.linkTaskCandidateDetail.bind(
                    this,
                    data._id,
                    candidate.user._id
                  )
                : this.linkTaskDetail.bind(this, data._id)
            }
            className="tableLink"
          >
            <Icon type="message" />
          </a>
        </Badge>
      </Tooltip>
    ) : null
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
            onClick={this.linkTaskDetail.bind(this, data._id)}
            className="tableLink"
          >
            <Icon type="message" />
          </a>
        </Badge>
      </Tooltip>
    ) : null
  }

  ord_renderContent() {
    const searchChangedHandler = e => {
      const search = e.target.value
      this.setState(
        {
          search,
          page: 1
        },
        this.debouncedRefetch
      )
    }

    return (
      <div className="p_ProfileTasks">
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_admin_content">
              <Row>
                <Col sm={24} md={4} className="wrap-box-navigator">
                  <Navigator selectedItem="profileTasks" />
                </Col>
                <Col
                  sm={24}
                  md={20}
                  className="c_ProfileContainer admin-right-column wrap-box-user"
                >
                  {(this.props.is_leader || this.props.is_admin) && (
                    <div className="pull-right filter-group btn-create-task">
                      <Button
                        onClick={() => this.props.history.push('/task-create/')}
                      >
                        {I18N.get('profile.tasks.create.task')}
                      </Button>
                    </div>
                  )}
                  <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
                    <Select
                      name="type"
                      onChange={this.onSelectFilter.bind(this)}
                      value={this.state.filter}
                    >
                      {_.map(FILTERS, (filter, key) => {
                        return (
                          <Select.Option key={filter} value={filter}>
                            {I18N.get(`profile.tasks.filter.${filter}`)}
                          </Select.Option>
                        )
                      })}
                    </Select>
                  </MediaQuery>
                  {!this.props.is_admin && (
                    <MediaQuery minWidth={MIN_WIDTH_PC}>
                      <Button.Group className="filter-group pull-left">
                        <Button
                          className={(this.state.filter === FILTERS.ALL && 'selected') || ''}
                          onClick={this.clearFilters.bind(this)}
                        >
                          {I18N.get('profile.tasks.filter.all')}
                        </Button>
                        {this.props.is_admin && (
                          <Button
                            className={(this.state.filter === FILTERS.NEED_APPROVAL && 'selected') || ''}
                            onClick={this.setNeedApprovalFilter.bind(this)}
                          >
                            {I18N.get('profile.tasks.filter.need_approval')}
                          </Button>
                        )}
                        <Button
                          className={(this.state.filter === FILTERS.OWNED && 'selected') || ''}
                          onClick={this.setOwnedFilter.bind(this)}
                        >
                          {I18N.get('profile.tasks.filter.owned')}
                        </Button>
                        <Button
                          className={(this.state.filter === FILTERS.ACTIVE && 'selected') || ''}
                          onClick={this.setActiveFilter.bind(this)}
                        >
                          {I18N.get('profile.tasks.filter.active')}
                        </Button>
                        <Button
                          className={
                            (this.state.filter === FILTERS.APPLIED && 'selected') || ''
                          }
                          onClick={this.setAppliedFilter.bind(this)}
                        >
                          {I18N.get('profile.tasks.filter.applied')}
                        </Button>
                        <Button
                          className={
                            (this.state.filter === FILTERS.SUBSCRIBED && 'selected') || ''
                          }
                          onClick={this.setSubscribedFilter.bind(this)}
                        >
                          {I18N.get('profile.tasks.filter.subscribed')}
                        </Button>
                      </Button.Group>
                    </MediaQuery>
                  )}
                  <div className="pull-right filter-group search-group">
                    <Input
                      defaultValue={this.state.search}
                      onChange={searchChangedHandler.bind(this)}
                      placeholder={I18N.get(
                        'developer.search.search.placeholder'
                      )}
                    />
                  </div>

                  {this.props.is_admin && (
                    <div className="pull-right status-selector">
                      <Select
                        showSearch={true}
                        allowClear={true}
                        placeholder="Select a status"
                        defaultValue={this.state.statusFilter}
                        onChange={this.setStatusFilter.bind(this)}
                      >
                        {_.keys(TASK_STATUS).map(taskStatus => {
                          return (
                            <Select.Option
                              key={taskStatus}
                              value={_.capitalize(taskStatus)}
                            >
                              {I18N.get(`taskStatus.${taskStatus}`)}
                            </Select.Option>
                          )
                        })}
                      </Select>
                    </div>
                  )}
                  <div className="clearfix" />
                  {this.renderList()}
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

  getCarousel(item) {
    const pictures = _.map(item.pictures, (picture, ind) => {
      return (
        <div key={ind}>
          <img width={188} height={188} alt="logo" src={picture.url} />
        </div>
      )
    })

    if (item.thumbnail) {
      pictures.unshift(
        <div key="main">
          <img width={188} height={188} alt="logo" src={item.thumbnail} />
        </div>
      )
    }

    if (_.isEmpty(pictures)) {
      pictures.unshift(
        <div key="main">
          <img
            width={188}
            height={188}
            alt="logo"
            src="/assets/images/Group_1685.12.svg"
          />
        </div>
      )
    }

    return (
      <div className="carousel-wrapper">
        <Carousel autoplay={true}>{pictures}</Carousel>
      </div>
    )
  }

  /**
   * This purposely only loads tasks from props because we are using pagination
   */
  renderList() {
    const tasks = this.props.all_tasks
    const description_fn = entity => {
      return (
        <div>
          {entity.applicationDeadline && (
            <div className="valign-wrapper">
              <div className="gap-right pull-left">
                {I18N.get('project.detail.deadline')}:
              </div>
              <div className="pull-left default-text">
                {moment(entity.applicationDeadline).format('MMM D')}
              </div>
            </div>
          )}
        </div>
      )
    }

    const data = _.map(tasks, (task, id) => {
      const applicationDeadline = task.applicationDeadline
        ? new Date(task.applicationDeadline).getTime()
        : Date.now()
      return {
        href: '',
        title: task.name,
        description: description_fn(task),
        content: task.description,
        owner: task.createdBy || {
          profile: {
            firstName: '',
            lastName: 'DELETED'
          }
        },
        applicationDeadlinePassed: Date.now() > applicationDeadline,
        id: task._id,
        status: task.status,
        task
      }
    })

    return (
      <List
        itemLayout="vertical"
        size="large"
        loading={this.props.loading || this.state.loadingMore}
        className="with-right-box"
        dataSource={data}
        pagination={{
          pageSize: this.state.results || 5,
          total: this.props.loading ? 0 : this.props.all_tasks_total,
          onChange: this.loadPage.bind(this)
        }}
        renderItem={item => (
          <div>
            <MediaQuery minWidth={MIN_WIDTH_PC}>
              <List.Item key={item.id} extra={this.getCarousel(item.task)}>
                <h3 className="no-margin no-padding one-line brand-color">
                  <a onClick={this.linkTaskDetail.bind(this, item.id)}>
                    {item.title}
                  </a>
                </h3>

                {/* Status */}
                <div className="valign-wrapper">
                  <Tag>
                    Status:
                    {item.status}
                  </Tag>
                </div>

                {item.applicationDeadlinePassed && (
                  <span className="subtitle">
                    {I18N.get('developer.search.subtitle_prefix')}{' '}
                    {I18N.get('developer.search.subtitle_applications')}
                  </span>
                )}
                <h5 className="no-margin">{item.description}</h5>
                <div className="description-content ql-editor">
                  <MarkdownPreview content={item.content} />
                </div>
                <div className="ant-list-item-right-box">
                  <a
                    className="pull-up"
                    onClick={this.linkUserDetail.bind(this, item.owner)}
                  >
                    <Avatar
                      size="large"
                      icon="user"
                      className="pull-right"
                      src={USER_AVATAR_DEFAULT}
                    />
                    <div className="clearfix" />
                    <div>
                      {item.owner.profile.firstName}{' '}
                      {item.owner.profile.lastName}
                    </div>
                  </a>
                  <Button
                    type="primary"
                    className="pull-down"
                    onClick={this.linkTaskDetail.bind(this, item.id)}
                  >
                    {I18N.get('profile.view')}
                    <div className="pull-right">
                      {this.props.page === 'LEADER' &&
                        this.getCommentStatus(item.task)}
                    </div>
                  </Button>
                </div>
              </List.Item>
            </MediaQuery>
            <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
              <List.Item key={item.id} className="ignore-right-box">
                <h3 className="no-margin no-padding one-line brand-color">
                  <a onClick={this.linkTaskDetail.bind(this, item.id)}>
                    {item.title}
                  </a>
                </h3>

                {/* Status */}
                <div className="valign-wrapper">
                  <Tag>
                    Status:
                    {item.status}
                  </Tag>
                </div>

                <h5 className="no-margin">{item.description}</h5>
                <div>
                  <a
                    onClick={this.linkUserDetail.bind(this, item.owner)}
                    className="owner-container"
                  >
                    <span>
                      {item.owner.profile.firstName}{' '}
                      {item.owner.profile.lastName}
                    </span>
                    <Divider type="vertical" />
                    <Avatar
                      size="large"
                      icon="user"
                      src={USER_AVATAR_DEFAULT}
                    />
                  </a>
                  <Button
                    type="primary"
                    className="pull-right view-button"
                    onClick={this.linkTaskDetail.bind(this, item.id)}
                  >
                    View
                    <div className="pull-right">
                      {this.props.page === 'LEADER' &&
                        this.getCommentStatus(item.task)}
                    </div>
                  </Button>
                </div>
              </List.Item>
            </MediaQuery>
          </div>
        )}
      />
    )
  }

  onSelectFilter(value) {
    const handlerLookupDefault = this.clearFilters
    const filters = [
      FILTERS.ACTIVE,
      FILTERS.APPLIED,
      FILTERS.SUBSCRIBED,
      FILTERS.OWNED,
      FILTERS.NEED_APPROVAL
    ]
    const handlers = [
      this.setActiveFilter,
      this.setAppliedFilter,
      this.setSubscribedFilter,
      this.setOwnedFilter,
      this.setNeedApprovalFilter
    ]
    const handlerLookup = _.zipObject(filters, handlers)
    const handler = handlerLookup[value] || handlerLookupDefault

    handler.call(this)
  }

  clearFilters() {
    this.setState(
      {
        filter: FILTERS.ALL,
        page: 1
      },
      this.refetch.bind(this)
    )
  }

  setActiveFilter() {
    this.setState(
      {
        filter: FILTERS.ACTIVE,
        page: 1
      },
      this.refetch.bind(this)
    )
  }

  setAppliedFilter() {
    this.setState(
      {
        filter: FILTERS.APPLIED,
        page: 1
      },
      this.refetch.bind(this)
    )
  }

  setOwnedFilter() {
    this.setState(
      {
        filter: FILTERS.OWNED,
        page: 1
      },
      this.refetch.bind(this)
    )
  }

  setSubscribedFilter() {
    this.setState(
      {
        filter: FILTERS.SUBSCRIBED,
        page: 1
      },
      this.refetch.bind(this)
    )
  }

  setNeedApprovalFilter() {
    this.setState(
      {
        filter: FILTERS.NEED_APPROVAL,
        page: 1
      },
      this.refetch.bind(this)
    )
  }

  // TODO: write documentation on how we save the filters between pages
  setStatusFilter(status) {
    this.props.setFilter({
      statusFilter: status
    })

    this.setState(
      {
        statusFilter: status
      },
      this.refetch.bind(this)
    )
  }

  linkTaskDetail(taskId) {
    this.props.history.push(`/task-detail/${taskId}`)
  }

  linkTaskCandidateDetail(taskId, taskCandidateId) {
    this.props.history.push(`/profile/task-app/${taskId}/${taskCandidateId}`)
  }

  linkUserDetail(user) {
    this.props.history.push(`/member/${user._id}`)
  }
}
