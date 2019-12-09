import React from 'react'
import Navigator from '@/module/page/shared/HomeNavigator/Container'
import _ from 'lodash'
import './style.scss'
import '../../admin/admin.scss'
import { Col, Row, Input, Button, Divider, Select, List, Carousel, Avatar, Tag } from 'antd'
import { TEAM_USER_STATUS, TEAM_AVATAR_DEFAULT } from '@/constant'
import MediaQuery from 'react-responsive'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import ProfilePage from '../../ProfilePage'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import {MAX_WIDTH_MOBILE, MIN_WIDTH_PC} from '../../../../config/constant'

const FILTERS = {
  ALL: 'all',
  OWNED: 'owned',
  ACTIVE: 'active',
  APPLIED: 'applied',
  REJECTED: 'rejected'
}

export default class extends ProfilePage {
  constructor(props) {
    super(props)

    this.debouncedLoadMore = _.debounce(this.loadMore.bind(this), 300)
    this.debouncedRefetch = _.debounce(this.refetch.bind(this), 300)

    this.state = {
      showMobile: false,
      filter: FILTERS.ALL,
      page: 1,
      results: 5,
      search: ''
    }
  }

  componentDidMount() {
    super.componentDidMount()
    this.refetch()
  }

  componentWillUnmount() {
    this.props.resetTeams()
  }

  getQuery() {
    let query = {}

    if (!this.props.is_admin) {
      query.teamHasUser = this.props.currentUserId
    }

    if (this.state.filter === FILTERS.ACTIVE) {
      query.teamHasUserStatus = TEAM_USER_STATUS.NORMAL
    }

    if (this.state.filter === FILTERS.APPLIED) {
      query.teamHasUserStatus = TEAM_USER_STATUS.PENDING
    }

    if (this.state.filter === FILTERS.REJECTED) {
      query.teamHasUserStatus = TEAM_USER_STATUS.REJECT
    }

    if (this.state.filter === FILTERS.OWNED) {
      query = {
        owner: this.props.currentUserId
      }
    }

    if (!_.isEmpty(this.state.search)) {
      query.search = this.state.search
    }

    query.page = this.state.page || 1
    query.results = this.state.results || 5

    return query
  }

  refetch() {
    const query = this.getQuery()
    this.props.getTeams(query)
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
      await this.props.loadMoreTeams(query)
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
      await this.props.loadMoreTeams(query)
      this.setState({ page })
    } catch (e) {
      // Do not update page in state if the call fails
    }

    this.setState({ loadingMore: false })
  }

  hasMoreTeams() {
    return _.size(this.props.all_teams) < this.props.all_teams_total
  }

  ord_renderContent () {
    const teams = this.props.all_teams
    const searchChangedHandler = (e) => {
      const search = e.target.value
      this.setState({
        search,
        page: 1
      }, this.debouncedRefetch)
    }

    return (
      <div className="p_ProfileTeams">
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_admin_content">
              <Row>
                <Col sm={24} md={4} className="wrap-box-navigator">
                  <Navigator selectedItem="profileTeams"/>
                </Col>
                <Col sm={24} md={20} className="c_ProfileContainer admin-right-column wrap-box-user">
                  {/* <div className="pull-right filter-group">
                    <Button onClick={this.goCreatepage.bind(this)}>{I18N.get('myrepublic.teams.create')}</Button>
                  </div> */}
                  <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
                    <Select
                      name="type"
                      onChange={this.onSelectFilter.bind(this)}
                      value={this.state.filter}
                    >
                      {_.map(FILTERS, (filter, key) => {
                        return (
                          <Select.Option key={filter} value={filter}>
                            {I18N.get(`myrepublic.teams.${filter}`)}
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
                        {I18N.get('myrepublic.teams.all')}
                      </Button>
                      <Button
                        className={(this.state.filter === FILTERS.OWNED && 'selected') || ''}
                        onClick={this.setOwnedFilter.bind(this)}>
                        {I18N.get('myrepublic.teams.owned')}
                      </Button>
                      <Button
                        className={(this.state.filter === FILTERS.ACTIVE && 'selected') || ''}
                        onClick={this.setActiveFilter.bind(this)}>
                        {I18N.get('myrepublic.teams.active')}
                      </Button>
                      <Button
                        className={(this.state.filter === FILTERS.APPLIED && 'selected') || ''}
                        onClick={this.setAppliedFilter.bind(this)}>
                        {I18N.get('myrepublic.teams.applied')}
                      </Button>
                      <Button
                        className={(this.state.filter === FILTERS.REJECTED && 'selected') || ''}
                        onClick={this.setRejectedFilter.bind(this)}>
                        {I18N.get('myrepublic.teams.rejected')}
                      </Button>
                    </Button.Group>
                  </MediaQuery>
                  <div className="pull-left filter-group search-group">
                    <Input defaultValue={this.state.search} onChange={searchChangedHandler.bind(this)}
                      placeholder={I18N.get('developer.search.search.placeholder')}/>
                  </div>
                  <div className="clearfix"/>
                  {this.getListComponent()}
                </Col>
              </Row>
            </div>
          </div>
        </div>
        <Footer/>
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

    return (
      <div className="carousel-wrapper">
        <Carousel autoplay={true}>
          {pictures}
        </Carousel>
      </div>
    )
  }

  getListItem(item) {
    const ownerProfile = item.owner && item.owner.profile || {}
    return (
      <div className="list-item">
        <MediaQuery minWidth={MIN_WIDTH_PC}>
          <List.Item
            key={item.id}
            extra={this.getCarousel(item)}
          >
            <h3 className="no-margin no-padding one-line brand-color">
              <a onClick={this.linkTeamDetail.bind(this, item.id)}>{item.title}</a>
            </h3>
            <h5 className="no-margin">
              {item.description}
            </h5>
            <div className="description-content ql-editor">
              <MarkdownPreview content={item.content} />
            </div>
            <div className="ant-list-item-right-box">
              <a className="pull-up" onClick={this.linkUserDetail.bind(this, item.owner)}>
                <Avatar size="large" icon="user" className="pull-right" src={ownerProfile.avatar}/>
                <div className="clearfix"/>
                <div>
                  <span className="no-info">
                    {I18N.get('task.owner')}
:
                    {' '}
                  </span>
                  {ownerProfile.firstName}
                  {' '}
                  {ownerProfile.lastName}
                </div>
              </a>
              <Button type="primary" className="pull-down" onClick={this.linkTeamDetail.bind(this, item.id)}>{I18N.get('profile.view')}</Button>
            </div>
          </List.Item>
        </MediaQuery>
        <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
          <List.Item
            key={item.id}
            className="ignore-right-box"
          >
            <h3 className="no-margin no-padding one-line brand-color">
              <a onClick={this.linkTeamDetail.bind(this, item.id)}>{item.title}</a>
            </h3>
            <h5 className="no-margin">
              {item.description}
            </h5>
            <div>
              <a onClick={this.linkUserDetail.bind(this, item.owner)}>
                <span>
                  {ownerProfile.firstName}
                  {' '}
                  {ownerProfile.lastName}
                </span>
                <Divider type="vertical"/>
                <Avatar size="large" icon="user" src={ownerProfile.avatar}/>
              </a>
              <Button
                type="primary"
                className="pull-right"
                onClick={this.linkTeamDetail.bind(this, item.id)}
              >
                {I18N.get('profile.view')}
              </Button>
            </div>
          </List.Item>
        </MediaQuery>
      </div>
    )
  }

  getListComponent() {
    const teams = this.props.all_teams
    const description_fn = (entity) => {
      return (
        <div className="valign-wrapper">
          <div className="gap-right pull-left">
            {I18N.get('project.detail.recruiting')}
:
            {' '}
          </div>
          <div className="pull-left">
            {_.isEmpty(entity.recruitedSkillsets) ? (
              <span>{I18N.get('project.detail.recruiting_skills_unknown')}</span>) : (
              _.map(entity.recruitedSkillsets, (skillset, ind) => <Tag key={ind}>{skillset}</Tag>))}
          </div>
        </div>
      )
    }

    const data = _.map(teams, (team, id) => {
      return {
        title: team.name,
        pictures: team.pictures && team.pictures.length > 0 ? team.pictures : [{ url: TEAM_AVATAR_DEFAULT }],
        description: description_fn(team),
        content: team.profile.description,
        owner: team.owner,
        id: team._id
      }
    })

    return (
      <List itemLayout="vertical" size="large" loading={this.props.loading || this.props.loadingMore}
        className="with-right-box" dataSource={data}
        pagination={{
          pageSize: this.state.results || 5,
          total: this.props.loading ? 0 : this.props.all_teams_total,
          onChange: this.loadPage.bind(this)
        }}
        renderItem={item => this.getListItem(item)}
      />
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
      case FILTERS.REJECTED:
        this.setRejectedFilter()
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

  setRejectedFilter() {
    this.setState({
      filter: FILTERS.REJECTED,
      page: 1
    }, this.refetch.bind(this))
  }

  setOwnedFilter() {
    this.setState({
      filter: FILTERS.OWNED,
      page: 1
    }, this.refetch.bind(this))
  }

  goCreatepage() {
    this.props.history.push('/profile/teams/create')
  }

  linkUserDetail(user) {
    this.props.history.push(`/member/${user._id}`)
  }

  linkTeamDetail(teamId) {
    this.props.history.push(`/team-detail/${teamId}`)
  }
}
