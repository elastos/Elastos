import React from 'react'
import _ from 'lodash'
import BaseComponent from '@/model/BaseComponent'
import UserEditForm from '@/module/form/UserEditForm/Container'
import I18N from '@/I18N'
import {Col, Row, Icon, Spin, Modal, Upload, Avatar} from 'antd'
import moment from 'moment-timezone'
import { upload_file } from '@/util'
import { getSafeUrl } from '@/util/url'
import {
  USER_AVATAR_DEFAULT,
  LINKIFY_OPTION,
  USER_ROLE_TO_TEXT,
} from '@/constant'
import config from '@/config'
import MediaQuery from 'react-responsive'
import linkifyStr from 'linkifyjs/string'
import sanitizeHtml from '@/util/html'
import GenderSvg from './GenderSvg'
import ProfileDid from './ProfileDid'
import TelegramIcon from '@/module/common/TelegramIcon'
import './style.scss'

/**
 * This has 3 views
 *
 * 1. Public
 * 2. Admin
 * 3. Edit
 *
 */
export default class extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      editing: false,
    }
  }

  // TODO: add twitter, telegram, linkedIn, FB
  ord_render() {
    if (_.isEmpty(this.props.user) || this.props.user.loading) {
      return (
        <div className="center">
          <Spin size="large" />
        </div>
      )
    }

    return (
      <div className="c_Member public">
        <div>
          <MediaQuery maxWidth={800}>
            <div className="member-content member-content-mobile">
              {this.renderMobile()}
            </div>
          </MediaQuery>
          <MediaQuery minWidth={801}>
            <div className="member-content">{this.renderDesktop()}</div>
          </MediaQuery>
        </div>
      </div>
    )
  }

  renderMobile() {
    return (
      <div>
        {this.renderBanner(true)}
        <div className="profile-info-container profile-info-container-mobile clearfix">
          {this.renderAvatar(true)}
          {this.renderFullName(true)}
          {this.renderDidBtn()}
          {this.renderRole(true)}
          {this.renderLocation(true)}
          {this.renderLocalTime(true)}
          {this.renderDescription(true)}
        </div>

        <div className="profile-info-container profile-info-container-mobile clearfix">
          <div className="pull-left skillset-header">
            <h4 className="komu-a">{I18N.get('profile.skillset.header')}</h4>
          </div>
          <div className="pull-right skillset-content">
            <Row>
              <Col span={24}>{this.renderSkillsets(true)}</Col>
              <Col span={24}>{this.renderProfession(true)}</Col>
            </Row>
          </div>
        </div>

        <div className="profile-info-container profile-info-container-mobile clearfix">
          <div className="pull-left skillset-header">
            <h4 className="komu-a">{I18N.get('profile.social.header')}</h4>
          </div>
          <div className="pull-right skillset-content">
            <Row>
              <Col span={24}>{this.renderSocialMedia(true)}</Col>
            </Row>
          </div>
        </div>

        {this.renderMetrics()}
        {this.renderEditForm()}
      </div>
    )
  }

  renderEditForm() {
    return (
      <Modal
        className="project-detail-nobar"
        visible={this.state.editing}
        onOk={this.switchEditMode.bind(this, false)}
        onCancel={this.switchEditMode.bind(this, false)}
        footer={null}
        width="70%"
      >
        {this.state.editing && (
          <UserEditForm
            user={this.props.user}
            switchEditMode={this.switchEditMode.bind(this, false)}
            completing={false}
          />
        )}
      </Modal>
    )
  }

  renderDesktop() {
    return (
      <div>
        {this.renderBanner()}
        <div className="profile-info-container clearfix">
          <div className="profile-left pull-left">{this.renderAvatar()}</div>
          <div className="profile-right pull-left">
            {this.renderFullName()}
            {this.renderDidBtn()}
            <Row>
              <Col span={24} className="profile-right-col">
                {this.renderRole()}
              </Col>
              <Col span={24} className="profile-right-col">
                {this.renderGender()}
              </Col>
              <Col span={24} className="profile-right-col">
                {this.renderLocation()}
              </Col>
              <Col span={24} className="profile-right-col">
                {this.renderLocalTime()}
              </Col>
            </Row>
          </div>
          <div className="clearfix" />
          {this.renderDescription()}
        </div>

        <div className="profile-info-container clearfix">
          <div className="pull-left skillset-header">
            <h4 className="komu-a">{I18N.get('profile.skillset.header')}</h4>
          </div>
          <div className="pull-right skillset-content">
            <Row>
              <Col span={14}>{this.renderSkillsets()}</Col>
              <Col span={10}>{this.renderProfession()}</Col>
            </Row>
          </div>
        </div>

        <div className="profile-info-container clearfix">
          <div className="pull-left skillset-header">
            <h4 className="komu-a">{I18N.get('profile.social.header')}</h4>
          </div>
          <div className="pull-right skillset-content">
            <Row>
              <Col span={24}>{this.renderSocialMedia()}</Col>
            </Row>
          </div>
        </div>

        {this.renderMetrics()}
        {this.renderEditForm()}
      </div>
    )
  }

  renderMetrics() {
    return (
      <Row gutter={16} className="profile-metrics">
        <Col md={24}>
          {this.renderMetricItem(
            I18N.get('profile.followers'),
            this.props.user.subscribers.length
          )}
        </Col>
      </Row>
    )
  }

  renderMetricItem(label, value) {
    return (
      <div className="item">
        <div className="value">{value}</div>
        <div className="center">{label}</div>
      </div>
    )
  }

  renderBanner(isMobile, url) {
    return (
      <div
        className={`profile-banner ${isMobile ? 'profile-banner-mobile' : ''}`}
      >
        <span
          style={{
            backgroundImage: this.getBannerWithFallback(
              url || this.props.user.profile.banner
            ),
          }}
        />
        <Icon
          className="profile-edit-btn"
          type="edit"
          onClick={this.switchEditMode.bind(this)}
        />
      </div>
    )
  }

  renderAvatar(isMobile) {
    const p_avatar = {
      showUploadList: false,
      customRequest: (info) => {
        upload_file(info.file).then(async (d) => {
          await this.props.updateUser(this.props.currentUserId, {
            profile: {
              avatar: d.url,
              avatarFilename: d.filename,
              avatarFileType: d.type,
            },
          })

          await this.props.getCurrentUser()
        })
      },
    }

    const renderAvatar = () => {
      const { avatar, firstName, lastName} = this.props.user.profile || {}

      if (avatar || (!firstName && !lastName)) {
        return (
          <Avatar
                src={avatar || USER_AVATAR_DEFAULT}
                shape="square"
                size={142}
            />
        )
      }

      if (firstName || lastName) {
        return (
          <Avatar
                style={{
                  backgroundColor: '#000',
                  fontSize: 64
                }}
                shape="square"
                size={142}
            >
            {`${firstName && firstName.toUpperCase().substr(0, 1)}${lastName && lastName.toUpperCase().substr(0, 1)}`}
          </Avatar>
        )
      }
    }

    return (
      <div
        className={`profile-avatar-container ${
          isMobile ? 'profile-avatar-container-mobile' : ''
        }`}
      >
        <div className="profile-avatar">
          <Upload
            name="avatar"
            listType="picture-card"
            className="avatar-uploader"
            {...p_avatar}
          >
            {this.props.avatar_loading ? (
              <div>
                <Icon type="loading" />
              </div>
            ) : (
              renderAvatar()
            )}
          </Upload>
        </div>
      </div>
    )
  }

  renderRole(isMobile) {
    const { user } = this.props
    return (
      <div
        className={`profile-general-info ${
          isMobile ? 'profile-general-info-mobile' : ''
        }`}
      >
        <Icon type="user" />
        <span>{user.role && USER_ROLE_TO_TEXT[user.role]}</span>
      </div>
    )
  }

  renderDidBtn() {
    const { getElaUrl, getNewActiveDid, user } = this.props
    return (
      <ProfileDid
        getElaUrl={getElaUrl}
        getNewActiveDid={getNewActiveDid}
        did={user.did}
      />
    )
  }

  renderFullName(isMobile) {
    return (
      <h1
        className={`komu-a profile-general-title ${
          isMobile ? 'profile-general-title-mobile' : ''
        }`}
      >
        {this.props.user.profile.firstName}
        &nbsp;
        {this.props.user.profile.lastName}
      </h1>
    )
  }

  renderLocation(isMobile) {
    return (
      <div
        className={`profile-general-info ${
          isMobile ? 'profile-general-info-mobile' : ''
        }`}
      >
        <Icon type="pushpin" />
        <span>{this.getCountryName(this.props.user.profile.country)}</span>
      </div>
    )
  }

  renderGender(isMobile) {
    return (
      <div
        className={`profile-general-info ${
          isMobile ? 'profile-general-info-mobile' : ''
        }`}
      >
        <GenderSvg />
        <span>{_.capitalize(this.props.user.profile.gender)}</span>
      </div>
    )
  }

  renderSkillsets(isMobile) {
    return (
      <div className="skillset-container">
        {_.map(this.props.user.profile.skillset || [], (skillset) => (
          <div key={skillset}>
+
            {I18N.get(`user.skillset.${skillset}`)}
          </div>
        ))}
      </div>
    )
  }

  renderProfession(isMobile) {
    return (
      <div className="profession-container">
        {this.props.user.profile.profession && (
          <div>
            {I18N.get(
              `profile.profession.${this.props.user.profile.profession}`
            )}
          </div>
        )}
        {!_.isEmpty(this.props.user.profile.portfolio) && (
          <div className="portfolio-container">
            <a
              href={getSafeUrl(this.props.user.profile.portfolio)}
              target="_blank"
              className="link-container"
            >
              <Icon type="link" />
              {' '}
              <span>
                {' '}
                {I18N.get('profile.portfolio')}
                {' '}
              </span>
            </a>
          </div>
        )}
      </div>
    )
  }

  getBannerWithFallback(banner) {
    return _.isEmpty(banner)
      ? "url('/assets/images/profile-banner.png')"
      : `url(${banner})`
  }

  getAvatarWithFallback(avatar) {
    return _.isEmpty(avatar) ? USER_AVATAR_DEFAULT : avatar
  }

  renderLocalTime(isMobile) {
    const now = moment(Date.now())
    const user = this.props.user
    const localTime = user.profile.timezone
      ? now.tz(user.profile.timezone).format('LT z')
      : 'Unknown'

    return (
      <div
        className={`profile-general-info ${
          isMobile ? 'profile-general-info-mobile' : ''
        }`}
      >
        <Icon type="clock-circle" />
        <span>
          {I18N.get('profile.localTime')}
          {' '}
          {localTime}
        </span>
      </div>
    )
  }

  renderSocialMedia(isMobile) {
    const { profile } = this.props.user

    return (
      <div
        className={`profile-social ${isMobile ? 'profile-social-mobile' : ''}`}
      >
        {profile.telegram && (
          <a href={getSafeUrl(profile.telegram)} target="_blank">
            <TelegramIcon style={{ fill: '#333333' }} />
          </a>
        )}
        {profile.twitter && (
          <a href={getSafeUrl(profile.twitter)} target="_blank">
            <Icon type="twitter" style={{ fontSize: 32 }} />
          </a>
        )}
        {profile.facebook && (
          <a href={getSafeUrl(profile.facebook)} target="_blank">
            <Icon type="facebook" style={{ fontSize: 32 }} />
          </a>
        )}
        {profile.reddit && (
          <a href={getSafeUrl(profile.reddit)} target="_blank">
            <Icon type="reddit" style={{ fontSize: 32 }} />
          </a>
        )}
        {profile.linkedin && (
          <a href={getSafeUrl(profile.linkedin)} target="_blank">
            <Icon type="linkedin" style={{ fontSize: 32 }} />
          </a>
        )}
        {profile.github && (
          <a href={getSafeUrl(profile.github)} target="_blank">
            <Icon type="github" style={{ fontSize: 32 }} />
          </a>
        )}
      </div>
    )
  }

  renderDescription(isMobile) {
    const bio = _.get(this.props, 'user.profile.bio') || ''
    const content = linkifyStr(bio, LINKIFY_OPTION)
    return (
      <div className="profile-container">
        {this.props.user.profile.bio && (
          <div
            className={`profile-description ${
              isMobile ? 'profile-description-mobile' : ''
            }`}
            dangerouslySetInnerHTML={{ __html: sanitizeHtml(content) }}
          />
        )}
      </div>
    )
  }

  getCountryName(countryCode) {
    return config.data.mappingCountryCodeToName[countryCode]
  }

  switchEditMode() {
    this.setState({
      editing: !this.state.editing,
    })
  }
}
