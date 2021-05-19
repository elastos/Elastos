import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Col,
  Row,
  Carousel,
  Button,
  Spin,
  Form,
  Modal
} from 'antd'
import { TEAM_USER_STATUS, USER_AVATAR_DEFAULT } from '@/constant'
import ProfilePopup from '@/module/profile/OverviewPopup/Container'
import _ from 'lodash'
import I18N from '@/I18N'
import { getSafeUrl } from '@/util/url'
import sanitizeHtml from '@/util/html'

import './style.scss'

/*
 * Project Pop-up UI
 *
 */
class C extends BaseComponent {

  ord_states() {
    return {
      showUserInfo: null
    }
  }

  componentDidMount() {
    const teamId = this.props.teamId
    this.props.getTeamDetail(teamId)
  }

  componentWillUnmount() {
    this.props.resetTeamDetail()
  }

  // Renderers
  ord_render() {
    const detail = this.props.detail
    const loading = this.props.loading || _.isEmpty(this.props.detail)

    return (
      <div className="c_TeamPopup">
        { loading
          ? (
            <div className="full-width full-height valign-wrapper halign-wrapper">
              <Spin className="loading-spinner" />
            </div>
          )
          : (
            <div>
              {this.renderHeader()}
              {this.getCarousel()}
              {this.renderMeta()}
              {/* {this.renderFooter()} */}
            </div>
          )
        }
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

  renderHeader() {
    return (
      <div className="app-header">
        <h3 className="komu-a with-gizmo">
          {this.props.detail.name}
        </h3>
      </div>
    )
  }

  renderMeta() {
    const generateRow = (key, value, cssRowClass) => (
      <Row className={[cssRowClass, 'app-meta-row'].join(' ')}>
        <Col span={8}>
          {key}
        </Col>
        <Col span={16}>
          {value}
        </Col>
      </Row>
    )
    const generateHtmlRow = (key, value, cssRowClass) => (
      <Row className={[cssRowClass, 'app-meta-row'].join(' ')}>
        <Col span={8}>
          {key}
        </Col>
        <Col span={16}>
          <div className="ql-editor" dangerouslySetInnerHTML={{__html: sanitizeHtml(value)}} />
        </Col>
      </Row>
    )

    const detail = this.props.detail

    return (
      <div className="app-meta">
        {generateRow(I18N.get('team.owner'), (
          <a onClick={this.linkProfileInfo.bind(this, detail.owner)}>
            {this.getUserNameWithFallback(detail.owner)}
          </a>
        ))}
        {generateHtmlRow(I18N.get('team.description'),
          detail.profile.description, 'team-description')}
      </div>
    )
  }

  renderFooter() {
    const detailUrl = `/team-detail/${this.props.detail._id}`
    return (
      <div className="app-footer valign-wrapper halign-wrapper">
        <Button href={getSafeUrl(detailUrl)}>
          {this.isTeamMember() || this.isTeamOwner()
            ? I18N.get('team.detail.view')
            : I18N.get('team.applyMessage')}
        </Button>
      </div>
    )
  }

  linkProfileInfo(user) {
    this.setState({
      showUserInfo: user
    })
  }

  getCarousel() {
    const IMAGE_SIZE = 150

    const item = this.props.detail
    const pictures = _.map(item.pictures, (picture, ind) => {
      return (
        <div key={ind}>
          <img width={IMAGE_SIZE} height={IMAGE_SIZE}
            alt="logo" src={picture.url} />
        </div>
      )
    })

    if (item.thumbnail) {
      pictures.unshift(
        <div key="main">
          <img width={IMAGE_SIZE} height={IMAGE_SIZE}
            alt="logo" src={item.thumbnail} />
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

  getAvatarWithFallback(avatar) {
    return _.isEmpty(avatar)
      ? USER_AVATAR_DEFAULT
      : avatar
  }

  getUserNameWithFallback(user) {
    if (_.isEmpty(user.profile.firstName) && _.isEmpty(user.profile.lastName)) {
      return user.username
    }

    return _.trim([user.profile.firstName, user.profile.lastName].join(' '))
  }

  isTeamOwner() {
    return this.props.detail.owner && (this.props.detail.owner._id === this.props.currentUserId)
  }

  isTeamMember() {
    return _.find(this.props.detail.members, (member) => {
      return member.user._id === this.props.currentUserId &&
                member.status === TEAM_USER_STATUS.NORMAL
    })
  }

  handleCancelProfilePopup() {
    this.setState({
      showUserInfo: null
    })
  }
}

export default Form.create()(C)
