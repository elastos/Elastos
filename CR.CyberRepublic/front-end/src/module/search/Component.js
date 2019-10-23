import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Col, Row, Icon, Input, Button, List, Checkbox, Radio, Select,
  Carousel, Modal, Avatar, Affix, Tag, TreeSelect, Switch, Divider, Spin
} from 'antd'
import _ from 'lodash'
import './style.scss'
import {SKILLSET_TYPE, TEAM_TASK_DOMAIN, TASK_CANDIDATE_STATUS, USER_AVATAR_DEFAULT, SORT_ORDER,
  TASK_CATEGORY} from '@/constant'
import InfiniteScroll from 'react-infinite-scroller'
import TeamDetail from '@/module/team/popup/Container'
import TaskDetail from '@/module/task/popup/Container'
import LoginOrRegisterForm from '@/module/form/LoginOrRegisterForm/Container'
import Footer from '@/module/layout/Footer/Container'
import MediaQuery from 'react-responsive'
import {MAX_WIDTH_MOBILE, MIN_WIDTH_PC} from '@/config/constant'
import I18N from '@/I18N'
import moment from 'moment'
import ProfilePopup from '@/module/profile/OverviewPopup/Container'
import URI from 'urijs'
import sanitizeHtml from '@/util/html'

const CheckboxGroup = Checkbox.Group
const RadioGroup = Radio.Group
const TreeNode = TreeSelect.TreeNode
const Option = Select.Option

export default class extends BaseComponent {
  constructor (props) {
    super(props)
    this.debouncedRefetch = _.debounce(this.refetch.bind(this), 300)
    this.debouncedLoadMore = _.debounce(this.loadMore.bind(this), 300)
  }

  componentDidMount() {
    this.props.loadAllCircles()
    this.refetch()
  }

  componentWillUnmount() {
    this.props.resetTeams()
    this.props.resetTasks()
  }

  ord_states() {
    const params = new URI(this.props.location.search || '').search(true)

    return {
      lookingFor: params.lookingFor || 'TEAM', // TEAM, PROJECT, TASK
      skillset: (params.skillset && params.skillset.split(',')) || [],
      domain: (params.domain && params.domain.split(',')) || [],
      circle: (params.circle && params.circle.split(',')) || [],
      search: params.search || '',
      entryCount: 3,
      skillsetShowAllEntries: false,
      categoryShowAllEntries: false,
      showTaskModal: false,
      showTeamModal: false,
      showLoginRegisterModal: false,
      taskDetailId: 0,
      teamDetailId: 0,
      showMobile: false,
      filtersTree: ['TEAM'],
      showUserInfo: null,
      page: 1,
      results: 5,
      sortBy: params.sortBy || 'createdAt',
      sortOrder: params.sortOrder || SORT_ORDER.DESC,
      assignment: params.assignment || 'all',
      taskCategory: params.taskCategory || 'all'
    }
  }

  getQuery() {
    const query = {}

    if (!_.isEmpty(this.state.search)) {
      query.search = this.state.search
    }

    if (!_.isEmpty(this.state.skillset)) {
      query.skillset = this.state.skillset
    }

    if (!_.isEmpty(this.state.domain)) {
      query.domain = this.state.domain
    }

    if (this.state.lookingFor === 'TASK') {
      if (!_.isEmpty(this.state.circle)) {
        query.circle = this.state.circle
      }
    }

    if (this.state.sortBy) {
      query.sortBy = this.state.sortBy
    }

    if (this.state.sortOrder) {
      query.sortOrder = this.state.sortOrder
    }

    if (this.state.assignment === 'unassigned') {
      query.unassigned = true
    }

    if (this.state.taskCategory && this.state.taskCategory !== 'all') {
      query.category = this.state.taskCategory
    }

    query.page = this.state.page || 1
    query.results = this.state.results || 5

    return query
  }

  getUrlForQuery(query) {
    const skillset = (query.skillset || []).join(',')
    const domain = (query.domain || []).join(',')
    const circle = (query.circle || []).join(',')
    const { lookingFor, search, sortBy, sortOrder, assignment, taskCategory } = this.state

    const url = new URI('/developer/search')
    lookingFor && url.addSearch('lookingFor', lookingFor)
    !_.isEmpty(skillset) && url.addSearch('skillset', skillset)
    !_.isEmpty(domain) && url.addSearch('domain', domain)
    !_.isEmpty(circle) && url.addSearch('circle', circle)
    !_.isEmpty(search) && url.addSearch('search', search)
    !_.isEmpty(sortBy) && url.addSearch('sortBy', sortBy)
    !_.isEmpty(sortOrder) && url.addSearch('sortOrder', sortOrder)
    assignment !== 'all' && url.addSearch('assignment', assignment)
    taskCategory !== 'all' && url.addSearch('taskCategory', taskCategory)

    return url.toString()
  }

  refetch() {
    const query = this.getQuery()
    const url = this.getUrlForQuery(query)
    const lookup = {
      TEAM: this.props.getTeams,
      PROJECT: this.props.getProjects,
      TASK: this.props.getTasks
    }

    const getter = lookup[this.state.lookingFor]
    getter.call(this, query)

    this.props.history.replace(url)
  }

  async loadMore() {
    const page = this.state.page + 1

    const query = {
      ...this.getQuery(),
      page,
      results: this.state.results
    }

    this.setState({ loadingMore: true })

    const lookup = {
      TASK: this.props.loadMoreTasks,
      TEAM: this.props.loadMoreTeams,
      PROJECT: this.props.loadMoreProjects
    }

    const getter = lookup[this.state.lookingFor]

    try {
      getter && await getter.call(this, query)
      this.setState({ page })
    } catch (e) {
      // Do not update page in state if the call fails
    }

    this.setState({ loadingMore: false })
  }

  hasMoreTasks() {
    return _.size(this.props.all_tasks) < this.props.all_tasks_total
  }

  hasMoreTeams() {
    return _.size(this.props.all_teams) < this.props.all_teams_total
  }

  hasMore() {
    const lookup = {
      TASK: this.hasMoreTasks,
      TEAM: this.hasMoreTeams,
      PROJECT: this.hasMoreTasks
    }

    const getter = lookup[this.state.lookingFor]
    return getter && getter.call(this)
  }

  // this needs to be used when it's a project to hide certain UI
  isLookingForTeam() {
    return this.state.lookingFor === 'TEAM'
  }

  isLookingForTask() {
    return this.state.lookingFor === 'TASK'
  }

  onChangeLookingFor(e) {
    this.setState({
      lookingFor: e.target.value,
      page: 1
    }, this.debouncedRefetch.bind(this))
  }

  onChangeSortBy(e) {
    this.setState({
      sortBy: e.target.value,
      page: 1
    }, this.debouncedRefetch.bind(this))
  }

  onChangeSortOrder(e) {
    this.setState({
      sortOrder: e.target.value,
      page: 1
    }, this.debouncedRefetch.bind(this))
  }

  onChangeSkillset(value) {
    this.setState({
      skillset: value,
      page: 1
    }, this.debouncedRefetch.bind(this))
  }

  onChangeDomain(value) {
    this.setState({
      domain: value,
      page: 1
    }, this.debouncedRefetch.bind(this))
  }

  onChangeCircle(value) {
    this.setState({
      circle: value,
      page: 1
    }, this.debouncedRefetch.bind(this))
  }

  onChangeLookingForSelect(value) {
    this.setState({
      lookingFor: value,
      page: 1
    }, this.debouncedRefetch.bind(this))
  }

  onChangeAssignment(e) {
    this.setState({
      assignment: e.target.value,
      page: 1
    }, this.debouncedRefetch.bind(this))
  }

  onChangeTaskCategory(e) {
    this.setState({
      taskCategory: e.target.value,
      page: 1
    }, this.debouncedRefetch.bind(this))
  }

  showTaskModal(id) {
    this.setState({
      showTaskModal: true,
      taskDetailId: id
    })
  }

  showTeamModal(id) {
    this.setState({
      showTeamModal: true,
      teamDetailId: id
    })
  }

  showLoginRegisterModal() {
    sessionStorage.setItem('loginRedirect', '/developer/search')
    sessionStorage.setItem('registerRedirect', '/developer/search')

    this.setState({
      showLoginRegisterModal: true
    })
  }

    handleTaskModalOk = (e) => {
      this.setState({
        showTaskModal: false
      })
    }

    handleTeamModalOk = (e) => {
      this.setState({
        showTeamModal: false
      })
    }

    handleTaskModalCancel = (e) => {
      this.setState({
        showTaskModal: false
      })
    }

    handleTeamModalCancel = (e) => {
      this.setState({
        showTeamModal: false
      })
    }

    handleLoginRegisterModalOk = (e) => {
      sessionStorage.removeItem('registerRedirect')

      this.setState({
        showLoginRegisterModal: false
      })
    }

    hideShowModal() {
      return () => {
        this.setState({
          showLoginRegisterModal: false
        })
      }
    }

    handleLoginRegisterModalCancel = (e) => {
      sessionStorage.removeItem('registerRedirect')

      this.setState({
        showLoginRegisterModal: false
      })
    }

    renderSortOptions() {
      return (
        <div>
          <RadioGroup value={this.state.sortBy}
            onChange={this.onChangeSortBy.bind(this)} className="gap-bottom">
            <Radio className="radio" value="createdAt">
              {I18N.get('developer.search.sort.createdAt')}
            </Radio>
            <Radio className="radio" value="updatedAt">
              {I18N.get('developer.search.sort.updatedAt')}
            </Radio>
          </RadioGroup>

          <RadioGroup value={this.state.sortOrder}
            onChange={this.onChangeSortOrder.bind(this)}>
            <Radio className="radio" value={SORT_ORDER.DESC}>
              {I18N.get('developer.search.sort.desc')}
            </Radio>
            <Radio className="radio" value={SORT_ORDER.ASC}>
              {I18N.get('developer.search.sort.asc')}
            </Radio>
          </RadioGroup>
        </div>
      )
    }

    renderAssignment() {
      return (
        <RadioGroup onChange={this.onChangeAssignment.bind(this)} value={this.state.assignment}>
          <Radio className="radio" value="all">
            {I18N.get('developer.search.assignment.all')}
          </Radio>
          <Radio className="radio" value="unassigned">
            {I18N.get('developer.search.assignment.unassigned')}
          </Radio>
        </RadioGroup>
      )
    }

    renderTaskCategory() {
      return (
        <RadioGroup onChange={this.onChangeTaskCategory.bind(this)} value={this.state.taskCategory}>
          <Radio className="radio" value="all">
            {I18N.get('developer.search.assignment.all')}
          </Radio>
          <Radio className="radio" value={TASK_CATEGORY.GENERAL}>
            {I18N.get('taks.application.general')}
          </Radio>
          <Radio className="radio" value={TASK_CATEGORY.DEVELOPER}>
            {I18N.get('taks.application.developer')}
          </Radio>
          <Radio className="radio" value={TASK_CATEGORY.SOCIAL}>
            {I18N.get('taks.application.social')}
          </Radio>
        </RadioGroup>
      )
    }

    renderLookingFor(lookingForOptions, showAll) {
      const limit = this.state.entryCount
      const filtered = _.take(lookingForOptions, showAll ? lookingForOptions.length : limit)
      const elements = _.map(filtered, (option) => {
        return (
          <Radio className="radio" value={option.value} key={option.value}>{option.label}</Radio>
        )
      })

      return (
        <RadioGroup onChange={this.onChangeLookingFor.bind(this)} value={this.state.lookingFor}>
          {elements}
        </RadioGroup>
      )
    }

    renderSkillset(skillsetOptions, showAll) {
      const limit = this.state.entryCount
      const filtered = _.take(skillsetOptions, showAll ? skillsetOptions.length : limit)
      const elements = _.map(filtered, (option) => {
        return (
          <div className="checkbox" key={option.value}>
            <Checkbox value={option.value}>
              {option.label}
            </Checkbox>
          </div>
        )
      })

      return (
        <CheckboxGroup onChange={this.onChangeSkillset.bind(this)} value={this.state.skillset}>
          {elements}
        </CheckboxGroup>
      )
    }

    renderCategory(categoryOptions, showAll) {
      const limit = this.state.entryCount
      const filtered = _.take(categoryOptions, showAll ? categoryOptions.length : limit)
      const elements = _.map(filtered, (option) => {
        return (
          <div className="checkbox" key={option.value}>
            <Checkbox value={option.value}>
              {option.label}
            </Checkbox>
          </div>
        )
      })

      return (
        <CheckboxGroup onChange={this.onChangeDomain.bind(this)} value={this.state.domain}>
          {elements}
        </CheckboxGroup>
      )
    }

    renderCircles(showAll) {
      const limit = this.state.entryCount
      const filtered = _.take(this.props.all_circles, showAll ? this.props.all_circles.length : limit)
      const elements = _.map(filtered, (circle) => {
        return (
          <div className="checkbox" key={circle._id}>
            <Checkbox value={circle._id}>
              {circle.name}
            </Checkbox>
          </div>
        )
      })

      return (
        <CheckboxGroup onChange={this.onChangeCircle.bind(this)} value={this.state.circle}>
          { this.props.all_circles_loading
            ? <Spin/>
            : elements
          }
        </CheckboxGroup>
      )
    }

    renderLoginOrRegisterModal() {
      return (
        <Modal
          className="project-detail-nobar"
          visible={this.state.showLoginRegisterModal}
          onOk={this.handleLoginRegisterModalOk}
          onCancel={this.handleLoginRegisterModalCancel}
          footer={null}
          width="70%"
        >
          <LoginOrRegisterForm onHideModal={this.hideShowModal()}/>
        </Modal>
      )
    }

    getSkillsetTree(skillsetOptions) {
      const filtered = _.take(skillsetOptions, skillsetOptions.length)
      const elements = _.map(filtered, (option) => {
        return (
          <TreeNode value={option.value} title={option.label} key={option.value}/>
        )
      })
      return elements
    }

    getCategoryTree(categoryOptions) {
      const filtered = _.take(categoryOptions, categoryOptions.length)
      const elements = _.map(filtered, (option) => {
        return (
          <TreeNode value={option.value} title={option.label} key={option.value}/>
        )
      })
      return elements
    }

    getCircleTree() {
      const elements = _.map(this.props.all_circles, (option) => {
        return {
          title: option.name,
          value: option._id,
          key: option._id
        }
      })

      return elements
    }

    handleOnFiltersChange(e) {
      const skillset = []
      const domain = []
      for (const item of e) {
        let found = this.getSkillsetOptions().find((option) => item === option.value)
        if (found) {
          skillset.push(found.value)
        }
        found = this.getCategoryOptions().find((option) => item === option.value)
        if (found) {
          domain.push(found.value)
        }
      }
      this.setState({
        filtersTree: e,
        skillset,
        domain,
        page: 1
      }, this.debouncedRefetch.bind(this))
    }

    getLookingForOptions() {
      return [
        {
          label: I18N.get('developer.search.team'),
          value: 'TEAM'
        },
        {
          label: I18N.get('developer.search.project'),
          value: 'PROJECT'
        },
        {
          label: I18N.get('developer.search.task'),
          value: 'TASK'
        }
      ]
    }

    getSkillsetOptions() {
      return [
        {
          label: 'C++',
          value: SKILLSET_TYPE.CPP
        },
        {
          label: 'JavaScript',
          value: SKILLSET_TYPE.JAVASCRIPT
        },
        {
          label: 'Go',
          value: SKILLSET_TYPE.GO
        },
        {
          label: 'Python',
          value: SKILLSET_TYPE.PYTHON
        },
        {
          label: 'Java',
          value: SKILLSET_TYPE.JAVA
        },
        {
          label: 'Swift',
          value: SKILLSET_TYPE.SWIFT
        }
      ]
    }

    getCategoryOptions() {
      return [
        {
          label: I18N.get('team.spec.authenticity'),
          value: TEAM_TASK_DOMAIN.AUTHENTICITY
        },
        {
          label: I18N.get('team.spec.currency'),
          value: TEAM_TASK_DOMAIN.CURRENCY
        },
        {
          label: I18N.get('team.spec.exchange'),
          value: TEAM_TASK_DOMAIN.EXCHANGE
        },
        {
          label: I18N.get('team.spec.finance'),
          value: TEAM_TASK_DOMAIN.FINANCE
        },
        {
          label: I18N.get('team.spec.gaming'),
          value: TEAM_TASK_DOMAIN.GAMING
        },
        {
          label: I18N.get('team.spec.iot'),
          value: TEAM_TASK_DOMAIN.IOT
        },
        {
          label: I18N.get('team.spec.media'),
          value: TEAM_TASK_DOMAIN.MEDIA
        },
        {
          label: I18N.get('team.spec.social'),
          value: TEAM_TASK_DOMAIN.SOCIAL
        },
        {
          label: I18N.get('team.spec.sovereignty'),
          value: TEAM_TASK_DOMAIN.SOVEREIGNTY
        }
      ]
    }

    getSidebarMenu() {
      const lookingForOptions = this.getLookingForOptions()
      const skillsetOptions = this.getSkillsetOptions()
      const categoryOptions = this.getCategoryOptions()

      const searchChangedHandler = (e) => {
        const search = e.target.value
        this.setState({
          search,
          page: 1
        }, this.debouncedRefetch)
      }

      return (
        <div>
          <MediaQuery minWidth={MIN_WIDTH_PC}>
            <Affix offsetTop={15}>
              <div className="group">
                <div className="content">
                  <Input defaultValue={this.state.search} onChange={searchChangedHandler.bind(this)}
                    placeholder={I18N.get('developer.search.search.placeholder')}/>
                </div>
              </div>
              {/*
                        <div className="group">
                            <div className="title">{I18N.get('developer.search.lookingFor')}</div>
                            <div className="content">
                                {this.renderLookingFor(lookingForOptions, true)}
                            </div>
                        </div>
                        */}
              <div className="group">
                <div className="title">{I18N.get('developer.search.sort')}</div>
                <div className="content">
                  {this.renderSortOptions()}
                </div>
              </div>

              {this.state.lookingFor === 'TASK' && (
              <div className="group">
                <div className="title">{I18N.get('developer.search.assignment')}</div>
                <div className="content">
                  {this.renderAssignment()}
                </div>
              </div>
              )}

              {this.state.lookingFor !== 'TASK' && (
              <div className="group">
                <div className="title">{I18N.get('developer.search.skillset')}</div>
                <div className="content">
                  {this.renderSkillset(skillsetOptions, this.state.skillsetShowAllEntries)}
                  {skillsetOptions.length > this.state.entryCount && (
                  <div className="showMore" onClick={this.enableSkillsetEntries.bind(this)}>
                    {
                                        !this.state.skillsetShowAllEntries ? (<span>{I18N.get('developer.search.showMore')}</span>)
                                          : (<span>{I18N.get('developer.search.hide')}</span>)
                                      }
                  </div>
                  )}
                </div>
              </div>
              )}
              {this.state.lookingFor !== 'TASK' && (
              <div className="group">
                <div className="title">{I18N.get('developer.search.category')}</div>
                <div className="content">
                  {this.renderCategory(categoryOptions, this.state.categoryShowAllEntries)}
                  { categoryOptions.length > this.state.entryCount && (
                  <div className="showMore" onClick={this.enableCategoryEntries.bind(this)}>
                    {
                                        !this.state.categoryShowAllEntries ? (<span>{I18N.get('developer.search.showMore')}</span>)
                                          : (<span>{I18N.get('developer.search.hide')}</span>)
                                      }
                  </div>
                  )}
                </div>
              </div>
              )}
              {this.state.lookingFor === 'TASK' && (
              <div className="group">
                <div className="title">{I18N.get('developer.search.taskCategory')}</div>
                <div className="content">
                  {this.renderTaskCategory()}
                </div>
              </div>
              )}
              {this.state.lookingFor === 'TASK' && (
              <div className="group">
                <div className="title">{I18N.get('developer.search.circle')}</div>
                <div className="content">
                  {this.props.all_circles_loading
                    ? <Spin/>
                    : (
                      <TreeSelect
                                    className="filters-tree"
                                    showSearch={true}
                                    value={this.state.circle}
                                    dropdownStyle={{ maxHeight: 400, overflow: 'auto' }}
                                    placeholder="Filters"
                                    allowClear={true}
                                    multiple={true}
                                    treeDefaultExpandAll={true}
                                    treeData={this.getCircleTree()}
                                    treeCheckable={!this.props.all_circles_loading}
                                    onChange={this.onChangeCircle.bind(this)}
                                  />
                    )
                                }
                </div>
              </div>
              )}
            </Affix>
          </MediaQuery>
          <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
            {/*
                    <div className="filter-select-container">
                        <Select className="filter-select" defaultValue={this.state.lookingFor} onChange={this.onChangeLookingForSelect.bind(this)}>
                            <Option value="PROJECT">{I18N.get('developer.search.project')}</Option>
                            <Option value="TEAM">{I18N.get('developer.search.team')}</Option>
                            <Option value="TASK">{I18N.get('developer.search.task')}</Option>
                        </Select>
                    </div>
                    */}
            <div className="search-mobile-container">
              <div className="">
                <Input defaultValue={this.state.search} onChange={searchChangedHandler.bind(this)}
                  placeholder={I18N.get('developer.search.search.placeholder')}/>
              </div>
            </div>
            {this.state.lookingFor !== 'TASK' && (
            <TreeSelect
                          className="filters-tree"
                          showSearch={true}
                          value={this.state.filtersTree}
                          dropdownStyle={{ maxHeight: 400, overflow: 'auto' }}
                          placeholder="Filters"
                          allowClear={true}
                          multiple={true}
                          treeDefaultExpandAll={true}
                          treeCheckable={true}
                          onChange={this.handleOnFiltersChange.bind(this)}
                        >
              <TreeNode value="0" title={I18N.get('developer.search.skillset')} key="0">
                {this.getSkillsetTree(skillsetOptions)}
              </TreeNode>
              <TreeNode value="1" title={I18N.get('developer.search.category')} key="1">
                {this.getCategoryTree(categoryOptions)}
              </TreeNode>
            </TreeSelect>
            )}
            {this.state.lookingFor === 'TASK' && (
            <TreeSelect
                      className="filters-tree"
                      showSearch={true}
                      value={this.state.circle}
                      dropdownStyle={{ maxHeight: 400, overflow: 'auto' }}
                      placeholder="Filters"
                      allowClear={true}
                      multiple={true}
                      treeDefaultExpandAll={true}
                      treeData={this.getCircleTree()}
                      treeCheckable={!this.props.all_circles_loading}
                      onChange={this.onChangeCircle.bind(this)}
                    />
            )}
          </MediaQuery>
        </div>
      )
    }

    enableSkillsetEntries() {
      this.setState({
        skillsetShowAllEntries: !this.state.skillsetShowAllEntries
      })
    }

    enableCategoryEntries() {
      this.setState({
        categoryShowAllEntries: !this.state.categoryShowAllEntries
      })
    }

    enableCirclesEntries() {
      this.setState({
        circlesShowAllEntries: !this.state.circlesShowAllEntries
      })
    }

    renderMain() {
      return (
        <div className="c_Search">
          <Row className="d_row">
            <Col sm={24} md={4} className="admin-left-column wrap-box-user">
              {this.getSidebarMenu()}
            </Col>
            <Col sm={24} md={20} className="admin-right-column wrap-box-user">
              {this.renderList()}
            </Col>
          </Row>
          <Modal
            className="project-detail-nobar"
            visible={this.state.showTaskModal}
            onOk={this.handleTaskModalOk}
            onCancel={this.handleTaskModalCancel}
            footer={null}
            width="70%"
          >
            { this.state.showTaskModal &&
            <TaskDetail taskId={this.state.taskDetailId}/>
            }
          </Modal>
          <Modal
            className="project-detail-nobar"
            visible={this.state.showTeamModal}
            onOk={this.handleTeamModalOk}
            onCancel={this.handleTeamModalCancel}
            footer={null}
            width="70%"
          >
            { this.state.showTeamModal &&
            <TeamDetail teamId={this.state.teamDetailId}/>
            }
          </Modal>
          <Modal
            className="profile-overview-popup-modal"
            visible={!!this.state.showUserInfo}
            onCancel={this.handleCancelProfilePopup.bind(this)}
            footer={null}>
            { this.state.showUserInfo &&
            <ProfilePopup showUserInfo={this.state.showUserInfo}/>
            }
          </Modal>
          {this.renderLoginOrRegisterModal()}
          <Footer/>
        </div>
      )
    }

    ord_render () {
      return this.renderMain()
    }

    getAvatarWithFallback(avatar) {
      return _.isEmpty(avatar)
        ? USER_AVATAR_DEFAULT
        : avatar
    }

    getCarousel(item) {
      const IMAGE_SIZE = 188
      const pictures = _.map(item.pictures, (picture, ind) => {
        return (
          <div key={ind}>
            <img width={IMAGE_SIZE} height={IMAGE_SIZE} alt="logo" src={picture.url} />
          </div>
        )
      })

      if (item.thumbnail) {
        pictures.unshift(
          <div key="main">
            <img width={IMAGE_SIZE} height={IMAGE_SIZE} alt="logo" src={item.thumbnail} />
          </div>
        )
      }

      if (pictures.length === 0) {
        pictures.push(<img width={IMAGE_SIZE} height={IMAGE_SIZE}
          src="/assets/images/Group_1685.12.svg" key={0} />)
      }

      return (
        <div className="carousel-wrapper">
          <Carousel autoplay={true}>
            {pictures}
          </Carousel>
        </div>
      )
    }

    renderList() {
      const entities = this.isLookingForTeam()
        ? this.props.all_teams
        : this.props.all_tasks

      const description_fn = (entity) => {
        return (
          <div>
            {!_.isEmpty(entity.recruitedSkillsets) && (
            <div className="valign-wrapper">
              <div className="gap-right pull-left">
                {I18N.get('project.detail.recruiting')}
:
                {' '}
              </div>
              <div className="pull-left">
                {_.map(entity.recruitedSkillsets, (skillset, ind) => <Tag key={ind}>{skillset}</Tag>)}
              </div>
            </div>
            )}
            {entity.bidding && (this.isAssigned(entity) ? (
              <div className="valign-wrapper">
                <div className="gap-right pull-left">
                  <h4 className="important-text">{I18N.get('project.detail.bid_selected')}</h4>
                </div>
              </div>
            ) : (entity.referenceBid && (
            <div className="valign-wrapper">
              <div className="gap-right pull-left">
                {I18N.get('project.detail.reference_bid')}
:
              </div>
              <div className="pull-left default-text">
                <b>
                  {entity.referenceBid}
                  {' '}
ELA
                </b>
              </div>
            </div>
            )))}
            {!entity.bidding && this.isAssigned(entity) && (
            <div className="valign-wrapper">
              <div className="gap-right pull-left">
                <h4 className="important-text">{I18N.get('project.detail.app_selected')}</h4>
              </div>
            </div>
            )}
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

      const data = this.isLookingForTeam()
        ? _.map(entities, (team, id) => {
          return {
            href: '',
            title: team.name,
            pictures: team.pictures,
            description: description_fn(team),
            content: team.profile.description,
            owner: team.owner,
            id: team._id
          }
        })
        : _.map(entities, (task, id) => {
          const applicationDeadline = task.applicationDeadline ? new Date(task.applicationDeadline).getTime() : null
          return {
            href: '',
            title: task.name,
            bidding: task.bidding,
            pictures: task.pictures,
            thumbnail: task.thumbnail,
            description: description_fn(task),
            content: task.description,
            owner: task.createdBy || {profile: {
              firstName: '',
              lastName: 'DELETED'
            }},
            status: task.status,
            applicationDeadlinePassed: applicationDeadline ? Date.now() > applicationDeadline : false,
            id: task._id,
            candidates: task.candidates
          }
        })

      const handlersLookup = {
        TEAM: this.showTeamModal,
        PROJECT: this.showTaskModal,
        TASK: this.showTaskModal
      }

      const clickHandler = !this.props.is_login
        ? this.showLoginRegisterModal
        : handlersLookup[this.state.lookingFor] || _.noop

      return (
        <InfiniteScroll
          initialLoad={false}
          pageStart={1}
          loadMore={this.debouncedLoadMore.bind(this)}
          hasMore={!this.state.loadingMore && !this.props.loading && this.hasMore()}
          useWindow={true}
        >
          <List loading={this.props.loading} itemLayout="vertical" size="large"
            className="with-right-box" dataSource={data}
            renderItem={item => (
              <div>
                <MediaQuery minWidth={MIN_WIDTH_PC}>
                  <List.Item
                    key={item.id}
                    extra={this.getCarousel(item)}
                  >
                    <h3 className="no-margin no-padding one-line brand-color task-title">
                      <a onClick={clickHandler.bind(this, item.id)}>{item.title}</a>
                    </h3>

                    {/* Status */}
                    { item.status && (
                    <div className="valign-wrapper">
                      <Tag>
                        {I18N.get('.status')}
:
                        {' '}
                        {item.status}
                      </Tag>
                    </div>
                    )}

                    {/* Application Deadline - info */}
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
                    <div className="description-ql-editor-wrapper">
                      <div className="ql-editor" dangerouslySetInnerHTML={{__html: sanitizeHtml(item.content)}}/>
                    </div>
                    <div className="ant-list-item-right-box">
                      <a className="pull-up" onClick={() => this.setState({ showUserInfo: item.owner })}>
                        <Avatar size="large" className="pull-right"
                          src={this.getAvatarWithFallback(item.owner.profile.avatar)}/>
                        <div className="clearfix"/>
                        <div>
                          {item.owner.profile.firstName}
                          {' '}
                          {item.owner.profile.lastName}
                        </div>
                      </a>

                      {/* {this.renderApplyButton(item, clickHandler)} */}

                    </div>
                  </List.Item>
                </MediaQuery>
                <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
                  <List.Item
                    key={item.id}
                    className="ignore-right-box"
                  >
                    <h3 className="no-margin no-padding one-line brand-color task-title">
                      <a onClick={clickHandler.bind(this, item.id)}>{item.title}</a>
                    </h3>

                    {/* Status */}
                    { item.status && (
                    <div className="valign-wrapper">
                      <Tag>
                        {I18N.get('.status')}
:
                        {' '}
                        {item.status}
                      </Tag>
                    </div>
                    )}

                    <h5 className="no-margin">
                      {item.description}
                    </h5>
                    <div>
                      <a onClick={() => this.setState({ showUserInfo: item.owner })}>
                        <span>
                          {item.owner.profile.firstName}
                          {' '}
                          {item.owner.profile.lastName}
                        </span>
                        <Divider type="vertical"/>
                        <Avatar size="large"
                          src={this.getAvatarWithFallback(item.owner.profile.avatar)}/>
                      </a>
                      <Button onClick={clickHandler.bind(this, item.id)}
                        type="primary" className="pull-right">
                        {I18N.get('developer.search.apply')}
                      </Button>
                    </div>
                  </List.Item>
                </MediaQuery>
              </div>
            )}
          >
            {this.state.loadingMore && this.hasMore() && (
            <div className="loadmore full-width halign-wrapper">
              <Spin />
            </div>
            )}
          </List>
        </InfiniteScroll>
      )
    }

    isAssigned(entity) {
      return !!_.find(entity.candidates, {status: TASK_CANDIDATE_STATUS.APPROVED})
    }

    handleCancelProfilePopup() {
      this.setState({
        showUserInfo: null
      })
    }

    // this is also just a view button if the project cannot accept anymore applications
    renderApplyButton(detail, clickHandler) {

      let cssClass = 'primary'

      if (detail.hasApprovedApplication) {
        cssClass = 'default'
      }

      return (
        <div className="pull-down">
          <span />
          <Button onClick={clickHandler.bind(this, detail.id)}
          type={cssClass}>
            {detail.hasApprovedApplication ?
              I18N.get('developer.search.view') : (
                detail.bidding ?
                  (this.isAssigned(detail) ? I18N.get('developer.search.view') : I18N.get('developer.search.submit_bid')) :
                  (this.isAssigned(detail) ? I18N.get('developer.search.view') : I18N.get('developer.search.apply'))
              )
          }
          </Button>
        </div>
      )
    }
}
