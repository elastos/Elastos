import React from 'react'
import ProfilePage from '../../ProfilePage'
import Footer from '@/module/layout/Footer/Container'
import Navigator from '@/module/page/shared/HomeNavigator/Container'
import _ from 'lodash'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import '../tasks/style.scss'
import '../../admin/admin.scss'
import {
  Col, Row, Icon, Badge, Tooltip, Button,
  Select, Divider, List, Carousel, Avatar, Tag, Modal, Input
} from 'antd'
import {TASK_CANDIDATE_STATUS, USER_AVATAR_DEFAULT, TASK_CATEGORY} from '@/constant'
import moment from 'moment/moment'
import MediaQuery from 'react-responsive'
import I18N from '@/I18N'
import {MAX_WIDTH_MOBILE, MIN_WIDTH_PC} from '../../../../config/constant'
import ProfilePopup from '@/module/profile/OverviewPopup/Container'

const FILTERS = {
  ALL: 'all',
  ACTIVE: 'active',
  APPLIED: 'applied',
  OWNED: 'owned',
  LIKED: 'liked',
  SUBSCRIBED: 'subscribed',
  CR100: 'cr100'
}

export default class extends ProfilePage {
  constructor(props) {
    super(props)

    this.state = {
      showMobile: false,
      filter: FILTERS.ALL,
      showUserInfo: null,
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

  getQuery() {
    const query = {
      profileListFor: this.props.currentUserId
    }

    // FILTERS.ALL - defaults to task.category [TASK_CATEGORY.DEVELOPER, TASK_CATEGORY.SOCIAL, TASK_CATEGORY.GENERAL]

    if (this.state.filter === FILTERS.ACTIVE) {
      query.taskHasUserStatus = TASK_CANDIDATE_STATUS.APPROVED
    }

    if (this.state.filter === FILTERS.APPLIED) {
      query.taskHasUserStatus = TASK_CANDIDATE_STATUS.PENDING
    }

    if (this.state.filter === FILTERS.CR100) {
      query.category = TASK_CATEGORY.CR100
    }

    if (this.state.filter === FILTERS.OWNED) {
      query.createdBy = this.props.currentUserId
    }

    if (this.state.filter === FILTERS.SUBSCRIBED) {
      query.subscribed = true
    }

    if (!_.isEmpty(this.state.search)) {
      query.search = this.state.search
    }

    query.page = this.state.page || 1
    query.results = this.state.results || 5

    console.log(query)

    return query
  }

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
      await this.props.loadMoreProjects(query)
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
      await this.props.loadMoreProjects(query)
      this.setState({ page })
    } catch (e) {
      // Do not update page in state if the call fails
    }

    this.setState({ loadingMore: false })
  }

  hasMoreProjects() {
    return _.size(this.props.all_tasks) < this.props.all_tasks_total
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
          <img width={188} height={188} alt="logo" src="/assets/images/Group_1685.12.svg" />
        </div>
      )
    }

    return (
      <div className="carousel-wrapper">
        <Carousel autoplay={true}>
          {pictures}
        </Carousel>
      </div>
    )
  }

  getListComponent() {
    const tasks = this.props.all_tasks
    const description_fn = (entity) => {
      return (
        <div>
          <div className="valign-wrapper">
            <div className="gap-right pull-left">
              {I18N.get('project.detail.recruiting')}
:
              {' '}
            </div>
            <div className="pull-left">
              {_.isEmpty(entity.recruitedSkillsets) ? (
                <span className="default-text">{I18N.get('project.detail.recruiting_skills_unknown')}</span>) : (
                _.map(entity.recruitedSkillsets, (skillset, ind) => <Tag key={ind}>{skillset}</Tag>))}
            </div>
          </div>
          {entity.applicationDeadline && (
          <div className="valign-wrapper">
            <div className="gap-right pull-left">
              {I18N.get('project.detail.deadline')}
:
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
      const applicationDeadline = task.applicationDeadline ? new Date(task.applicationDeadline).getTime() : Date.now()
      return {
        href: '',
        title: task.name,
        description: description_fn(task),
        content: task.description,
        owner: task.createdBy || {profile: {
          firstName: '',
          lastName: 'DELETED'
        }},
        applicationDeadlinePassed: Date.now() > applicationDeadline,
        id: task._id,
        status: task.status,
        task
      }
    })

    return (
      <List itemLayout="vertical" size="large" loading={this.props.loading || this.props.loadingMore}
        className="with-right-box" dataSource={data}
        pagination={{
          pageSize: this.state.results || 5,
          total: this.props.loading ? 0 : this.props.all_tasks_total,
          onChange: this.loadPage.bind(this)
        }}
        renderItem={item => (
          <div>
            <MediaQuery minWidth={MIN_WIDTH_PC}>
              <List.Item
                key={item.id}
                extra={this.getCarousel(item.task)}
              >
                <h3 className="no-margin no-padding one-line brand-color">
                  <a onClick={this.linkTaskDetail.bind(this, item.task)}>{item.title}</a>
                </h3>

                {/* Status */}
                <div className="valign-wrapper">
                  <Tag>
                    {I18N.get('admin.tasks.status')}:{' '}
                    {I18N.get(`taskStatus.${item.status}`)}
                  </Tag>
                </div>

                {item.applicationDeadlinePassed && (
                <span className="subtitle">
                  {I18N.get('developer.search.subtitle_prefix')}
                  {' '}
                  {I18N.get('developer.search.subtitle_applications')}
                </span>
                )}
                <h5 className="no-margin">
                  {item.description}
                </h5>
                <div className="description-content ql-editor">
                  <MarkdownPreview content={item.content} />
                </div>
                <div className="ant-list-item-right-box">
                  <a className="pull-up" onClick={this.linkUserDetail.bind(this, item.owner)}>
                    <Avatar size="large" icon="user" className="pull-right" src={USER_AVATAR_DEFAULT}/>
                    <div className="clearfix"/>
                    <div>
                      {item.owner.profile.firstName}
                      {' '}
                      {item.owner.profile.lastName}
                    </div>
                  </a>
                  <Button type="primary" className="pull-down" onClick={this.linkTaskDetail.bind(this, item.task)}>
                                        View
                    <div className="pull-right">
                      {this.props.page === 'LEADER' && this.getCommentStatus(item.task)}
                    </div>
                  </Button>
                </div>
              </List.Item>
            </MediaQuery>
            <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
              <List.Item
                key={item.id}
                className="ignore-right-box"
              >
                <h3 className="no-margin no-padding one-line brand-color">
                  <a onClick={this.linkTaskDetail.bind(this, item.task)}>{item.title}</a>
                </h3>

                {/* Status */}
                <div className="valign-wrapper">
                  <Tag>
Status:
                    {item.status}
                  </Tag>
                </div>

                <h5 className="no-margin">
                  {item.description}
                </h5>
                <div>
                  <a onClick={this.linkUserDetail.bind(this, item.owner)}>
                    <span>
                      {item.owner.profile.firstName}
                      {' '}
                      {item.owner.profile.lastName}
                    </span>
                    <Divider type="vertical"/>
                    <Avatar size="large" icon="user" src={USER_AVATAR_DEFAULT}/>
                  </a>
                  <Button type="primary" className="pull-right" onClick={this.linkTaskDetail.bind(this, item.task)}>
                                        View
                    <div className="pull-right">
                      {this.props.page === 'LEADER' && this.getCommentStatus(item.task)}
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

  getCandidateUnreadMessageCount(task) {
    const candidate = _.find(task.candidates, (candidate) => {
      return candidate.user && candidate.user._id === this.props.currentUserId
    })
    let unread = []

    if (candidate) {
      const lastDate = candidate.lastSeenByCandidate
      unread = _.filter(candidate.comments, (comment) => {
        return !lastDate || new Date(_.first(comment).createdAt) > new Date(lastDate)
      })
    } else {
      unread = _.flatten(_.map(task.candidates, (candidate) => {
        const lastDate = candidate.lastSeenByOwner
        const subUnread = _.filter(candidate.comments, (comment) => {
          return !lastDate || new Date(_.first(comment).createdAt) > new Date(lastDate)
        })
        return subUnread
      }))
    }

    return _.size(unread)
  }

  getUnreadMessageCount(task) {
    const isOwner = task.createdBy && task.createdBy._id === this.props.currentUserId
    const subscription = _.find(task.subscribers, (subscriber) => {
      return subscriber.user && subscriber.user._id === this.props.currentUserId
    })
    const lastDate = isOwner
      ? task.lastCommentSeenByOwner
      : subscription && subscription.lastSeen

    const unread = _.filter(task.comments, (comment) => {
      return !lastDate || new Date(_.first(comment).createdAt) > new Date(lastDate)
    })

    return _.size(unread)
  }

  getCommentStatus(task) {
    const unread = this.getUnreadMessageCount(task) + this.getCandidateUnreadMessageCount(task)
    const tooltipSuffix = unread > 1 ? 's' : ''
    const tooltip = `${unread} new message${tooltipSuffix}`

    return unread.length
      ? (
        <Tooltip title={tooltip}>
          <Badge dot={true} count={unread}>
            <a onClick={this.linkTaskDetail.bind(this, task)} className="tableLink">
              <Icon type="message" className="white"/>
            </a>
          </Badge>
        </Tooltip>
      )
      : null
  }

  ord_renderContent () {
    const searchChangedHandler = (e) => {
      const search = e.target.value
      this.setState({
        search,
        page: 1
      }, this.debouncedRefetch)
    }

    return (
      <div className="p_ProfileProjects">
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_admin_content">
              <Row>
                <Col sm={24} md={4} className="wrap-box-navigator">
                  <Navigator selectedItem="profileProjects"/>
                </Col>
                <Col sm={24} md={20} className="c_ProfileContainer admin-right-column wrap-box-user">
                  {(this.props.is_leader || this.props.is_admin) && (
                  <div className="pull-right filter-group">
                    <Button onClick={() => this.props.history.push('/task-create?type=PROJECT&category=DEVELOPER')}>{I18N.get('myrepublic.projects.create')}</Button>
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
                            {I18N.get(`myrepublic.projects.${filter}`)}
                          </Select.Option>
                        )
                      })}
                    </Select>
                  </MediaQuery>
                  <MediaQuery minWidth={MIN_WIDTH_PC}>
                    <Button.Group className="filter-group pull-left">
                      <Button
                        className={(this.state.filter === FILTERS.ALL && 'selected') || ''}
                        onClick={this.clearFilters.bind(this)}>
                        {I18N.get('myrepublic.projects.all')}
                      </Button>
                      <Button
                        className={(this.state.filter === FILTERS.OWNED && 'selected') || ''}
                        onClick={this.setOwnedFilter.bind(this)}>
                        {I18N.get('myrepublic.projects.owned')}
                      </Button>
                      <Button
                        className={(this.state.filter === FILTERS.ACTIVE && 'selected') || ''}
                        onClick={this.setActiveFilter.bind(this)}>
                        {I18N.get('myrepublic.projects.active')}
                      </Button>
                      <Button
                        className={(this.state.filter === FILTERS.APPLIED && 'selected') || ''}
                        onClick={this.setAppliedFilter.bind(this)}>
                        {I18N.get('myrepublic.projects.applied')}
                      </Button>
                      <Button
                        className={(this.state.filter === FILTERS.SUBSCRIBED && 'selected') || ''}
                        onClick={this.setSubscribedFilter.bind(this)}>
                        {I18N.get('myrepublic.projects.liked')}
                      </Button>
                      <Button
                        className={(this.state.filter === FILTERS.CR100 && 'selected') || ''}
                        onClick={this.setCr100Filter.bind(this)}>
                        {I18N.get('myrepublic.projects.cr100')}
                      </Button>
                    </Button.Group>
                  </MediaQuery>
                  <div className="pull-left filter-group search-group">
                    <Input defaultValue={this.state.search} onChange={searchChangedHandler.bind(this)}
                      placeholder={I18N.get('developer.search.search.placeholder')}/>
                  </div>
                  <div className="clearfix"/>
                  {this.state.filter === FILTERS.ALL && this.getListComponent()}
                  {this.state.filter === FILTERS.ACTIVE && this.getListComponent()}
                  {this.state.filter === FILTERS.APPLIED && this.getListComponent()}
                  {this.state.filter === FILTERS.OWNED && this.getListComponent()}
                  {this.state.filter === FILTERS.SUBSCRIBED && this.getListComponent()}
                  {this.state.filter === FILTERS.CR100 && this.getListComponent()}
                </Col>
              </Row>
              <Row>
                <Col>
                  <br/>
                </Col>
              </Row>
            </div>
          </div>
        </div>
        <Footer/>
        <Modal
          className="profile-overview-popup-modal"
          visible={!!this.state.showUserInfo}
          onCancel={this.handleCancelProfilePopup.bind(this)}
          footer={null}>
          { this.state.showUserInfo &&
          <ProfilePopup showUserInfo={this.state.showUserInfo}/>
          }
        </Modal>
      </div>
    )
  }

  onSelectFilter(value) {
    switch (value) {
      case FILTERS.ACTIVE:
        this.setActiveFilter()
        break
      case FILTERS.APPLIED:
        this.setAppliedFilter()
        break
      case FILTERS.SUBSCRIBED:
        this.setSubscribedFilter()
        break
      case FILTERS.OWNED:
        this.setOwnedFilter()
        break
      default:
        this.clearFilters()
        break
    }
  }

  clearFilters() {
    this.setState({
      filter: FILTERS.ALL,
      page: 1
    }, this.refetch.bind(this))
  }

  setActiveFilter() {
    this.setState({
      filter: FILTERS.ACTIVE,
      page: 1
    }, this.refetch.bind(this))
  }

  setAppliedFilter() {
    this.setState({
      filter: FILTERS.APPLIED,
      page: 1
    }, this.refetch.bind(this))
  }

  setOwnedFilter() {
    this.setState({
      filter: FILTERS.OWNED,
      page: 1
    }, this.refetch.bind(this))
  }

  setSubscribedFilter() {
    this.setState({
      filter: FILTERS.SUBSCRIBED,
      page: 1
    }, this.refetch.bind(this))
  }

  setCr100Filter() {
    this.setState({
      filter: FILTERS.CR100,
      page: 1
    }, this.refetch.bind(this))
  }

  linkTaskDetail(task) {
    this.props.history.push(task.category === TASK_CATEGORY.CR100
      ? `/project-detail/${task._id}`
      : `/task-detail/${task._id}`)
  }

  linkTaskCandidateDetail(taskId, taskCandidateId) {
    this.props.history.push(`/profile/task-app/${taskId}/${taskCandidateId}`)
  }

  linkUserDetail(user) {
    this.setState({
      showUserInfo: user
    })
  }

  handleCancelProfilePopup() {
    this.setState({
      showUserInfo: null
    })
  }
}
