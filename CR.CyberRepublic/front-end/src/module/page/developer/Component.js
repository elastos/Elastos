import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import Video from '@/module/shared/video/Container'
import _ from 'lodash'
import './style.scss'
import { Col, Row, Table, Input, Modal, Avatar, TreeSelect } from 'antd'
import { CR_LINKS, USER_AVATAR_DEFAULT, USER_SKILLSET, USER_PROFESSION } from '@/constant'
import ProfilePopup from '@/module/profile/OverviewPopup/Container'
import URI from 'urijs'
import StandardPage from '../StandardPage'

export default class extends StandardPage {
  constructor(props) {
    super(props)

    const params = new URI(props.location.search || '').search(true)

    this.state = {
      search: params.search || '',
      showUserInfo: null,
      skillsetFilters: (params.skillset && params.skillset.split(',')) || [],
      professionFilters: (params.profession && params.profession.split(',')) || [],
      showVideo: false,
      userListPagination: {
        pageSize: 5,
        current: parseInt(params.page || 1, 10)
      }
    }
  }

  async componentDidMount() {
    this.refetch()
    this.debouncedRefetch = _.debounce(this.refetch.bind(this), 300)
  }

  componentWillUnmount() {
  }

  refetch() {
    const options = {
      search: this.state.search || '',
      results: (this.state.userListPagination || {}).pageSize || 5,
      page: (this.state.userListPagination || {}).current || 1,
      skillset: this.state.skillsetFilters || [],
      profession: this.state.professionFilters || []
    }

    const url = new URI('/developer')
    if (!_.isEmpty(this.state.search)) {
      url.addSearch('search', this.state.search)
    }
    if (options.page > 1) {
      url.addSearch('page', options.page)
    }
    if (options.skillset.lengtg) {
      url.addSearch('skillset', options.skillset.join(','))
    }
    if (options.profession.length) {
      url.addSearch('profession', options.profession.join(','))
    }

    this.props.history.replace(url.toString())
    this.props.listUsers(options)
  }

  ord_renderContent () {
    return (
      <div className="p_Developer">
        <div className="ebp-header-divider" />
        <div className="p_admin_index">
          <div className="d_box">
            <div className="p_admin_content">
              {this.buildInfoPanel()}
              {this.buildNavi()}
              {/* this.buildMemberSearch() */}
            </div>
          </div>
        </div>
        {this.renderVideoModal()}
        {this.renderProfileModal()}
        <Footer/>
      </div>
    )
  }

  renderVideoModal() {
    return (
      <Modal
        className="video-popup-modal"
        visible={!!this.state.showVideo}
        onCancel={this.handleCancelVideo.bind(this)}
        footer={null}
        width="76%"
        style={{ top: 35 }}>
        <Video showVideo={this.state.showVideo} />
      </Modal>
    )
  }

  renderProfileModal() {
    return (
      <Modal
        className="profile-overview-popup-modal"
        visible={!!this.state.showUserInfo}
        onCancel={this.handleCancelProfilePopup.bind(this)}
        footer={null}>
        { this.state.showUserInfo &&
        <ProfilePopup showUserInfo={this.state.showUserInfo}/>
        }
      </Modal>
    )
  }

  handleCancelVideo() {
    this.setState({
      showVideo: false
    })
  }

  handleShowVideo() {
    this.setState({
      showVideo: true
    })
  }

  handleCancelProfilePopup() {
    this.setState({
      showUserInfo: null
    })
  }

  buildInfoPanel() {
    return (
      <div className="info-panel panel">
        <div className="info-panel-content panel-content">
          <div className="info-panel-left pull-left">
            <h3 className="with-gizmo">
              {I18N.get('0002')}
            </h3>
            <div className="info-panel-link">
              <a href={CR_LINKS.TELEGRAM} target="_blank">
                                Telegram
              </a>
            </div>
            <div className="info-panel-link">
              <a href={CR_LINKS.TWITTER} target="_blank">
                                Twitter
              </a>
            </div>
            <div className="info-panel-link">
              <a href={CR_LINKS.GITHUB} target="_blank">
                                GitHub
              </a>
            </div>
            <div className="info-panel-link">
              <a href={`${process.env.FORUM_URL}/login`} target="_blank">
                                Forum
              </a>
            </div>
          </div>
          <div className="video" onClick={this.handleShowVideo.bind(this)}>
            <a className="video-btn">
              <p className="video-title">Cyber Republic Video</p>
              <p className="video-more">Watch it now and learn more</p>
              <img className="video-img" src="/assets/images/arrow-right.png"/>
            </a>
          </div>
          <div className="pull-right">
            <img src="/assets/images/community-world.svg"/>
          </div>
          <div className="clearfix"/>
        </div>
      </div>
    )
  }

  buildNavi() {
    const buildNaviItem = (title, description, link) => {
      return (
        <Row gutter={24} className="navi-panel-item"
          onClick={() => this.props.history.push(link)}>
          <Col md={8} xs={24} className="navi-panel-item-title">
            {title}
          </Col>
          <Col md={12} xs={24} className="navi-panel-item-description">
            {description}
          </Col>
          <Col md={4} xs={24} className="navi-panel-item-arrow">
            <img src="/assets/images/arrow-right.png"/>
          </Col>
        </Row>
      )
    }

    return (
      <div className="navi-panel panel">
        <div className="navi-panel-content panel-content">
          {buildNaviItem(I18N.get('developer.teams.title'), I18N.get('developer.teams.description'), '/developer/search')}
          {buildNaviItem(I18N.get('developer.learn'), I18N.get('developer.learn.description'), '/developer/learn')}
        </div>
      </div>
    )
  }

  showUserProfile(user) {
    this.setState({
      showUserInfo: user
    })
  }

  getUserClickableLink(user, name) {
    return <a onClick={this.showUserProfile.bind(this, user)}>{name}</a>
  }

  getUserNameWithFallback(user) {
    const name = _.isEmpty(user.profile.firstName) && _.isEmpty(user.profile.lastName)
      ? user.username
      : _.trim([user.profile.firstName, user.profile.lastName].join(' '))
    return this.getUserClickableLink(user, name)
  }

  getUserCircles(user) {
    if (_.isEmpty(user.circles)) {
      return ''
    }

    return _.map(user.circles, (circle) => (
      <a key={circle._id} href={`/crcles-detail/${circle._id}`}>
        {circle.name}
        {' '}
      </a>
    ))
  }

  getAvatarWithFallback(avatar) {
    return _.isEmpty(avatar)
      ? USER_AVATAR_DEFAULT
      : avatar
  }

  skillsetFilterChanged(value) {
    this.setState({ skillsetFilters: value }, this.debouncedRefetch.bind(this))
  }

  professionFilterChanged(value) {
    this.setState({ professionFilters: value }, this.debouncedRefetch.bind(this))
  }

  getSkillsets() {
    return _.map(USER_SKILLSET, (skillsets, category) => {
      return {
        title: I18N.get(`user.skillset.group.${category}`),
        value: category,
        key: category,
        children: _.map(_.keys(skillsets).sort(), (skillset) => {
          return {
            title: I18N.get(`user.skillset.${skillset}`),
            value: skillset,
            key: skillset
          }
        })
      }
    })
  }

  getSkillsetSelector() {
    return (
      <TreeSelect
        className="full-width"
        value={this.state.skillsetFilters}
        treeData={this.getSkillsets()}
        treeCheckable={true}
        searchPlaceholder={I18N.get('user.skillset.select')}
        onChange={this.skillsetFilterChanged.bind(this)}/>
    )
  }

  getProfessions() {
    // Make sure Other is the last entry
    const keys = _.union(_.without(_.keys(USER_PROFESSION).sort(), USER_PROFESSION.OTHER),
      [USER_PROFESSION.OTHER])

    return _.map(keys, (profession) => {
      return {
        title: I18N.get(`profile.profession.${profession}`),
        value: profession,
        key: profession
      }
    })
  }

  getProfessionSelector() {
    return (
      <TreeSelect
        className="full-width"
        value={this.state.professionFilters}
        treeData={this.getProfessions()}
        treeCheckable={true}
        searchPlaceholder={I18N.get('user.profession.select')}
        onChange={this.professionFilterChanged.bind(this)}/>
    )
  }

  handleTableChange(pagination, filters, sorter) {
    const pager = { ...this.state.userListPagination }
    pager.current = pagination.current
    this.setState({
      userListPagination: pager
    }, this.refetch.bind(this))
  }

  buildMemberSearch() {
    const columns = [
      {
        title: I18N.get('developer.member.table.column.member'),
        key: 'name',
        width: '33%',
        render: user => {
          return (
            <div>
              <Avatar className={`gap-right ${user.role === 'LEADER' ? 'avatar-leader' : 'avatar-member'}`}
                src={this.getAvatarWithFallback(user.profile.avatar)}/>
              {this.getUserNameWithFallback(user)}
            </div>
          )
        }
      }, {
        title: I18N.get('developer.member.table.column.username'),
        dataIndex: 'username',
        width: '33%',
        render: (username, user) => this.getUserClickableLink(user, user.username)
      }, {
        title: I18N.get('developer.member.table.column.circles'),
        dataIndex: 'circles',
        width: '33%',
        render: (circles, user) => this.getUserCircles(user)
      }
    ]

    const searchChangedHandler = (e) => {
      const search = e.target.value
      this.setState({
        search,
        userListPagination: {
          ...this.state.userListPagination,
          current: 1
        }
      }, this.debouncedRefetch)
    }

    return (
      <div className="member-panel panel">
        <div className="member-panel-content panel-content">
          <h3 className="with-gizmo">
            {I18N.get('developer.member.search.title')}
          </h3>
          <Row className="member-panel-search" gutter={16}>
            <Col md={8} xs={24}>
              <Input defaultValue={this.state.search} placeholder={I18N.get('developer.breadcrumb.search')}
                onChange={searchChangedHandler.bind(this)}/>
            </Col>
            <Col md={8} xs={24}>
              {this.getProfessionSelector()}
            </Col>
            <Col md={8} xs={24}>
              {this.getSkillsetSelector()}
            </Col>
          </Row>
          <Table
            className="no-borders"
            dataSource={this.props.users}
            loading={this.props.loading}
            columns={columns}
            bordered={false}
            rowKey="_id"
            pagination={{
              ...this.state.userListPagination,
              total: this.props.users_total,
              showTotal: total => `Total ${total} users`
            }}
            onChange={this.handleTableChange.bind(this)} />
        </div>
      </div>
    )
  }
}
