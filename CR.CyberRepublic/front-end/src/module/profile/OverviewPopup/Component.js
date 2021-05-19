import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import UserContactForm from '@/module/form/UserContactForm/Container'
import {
  Button, Icon, Modal, Spin,
} from 'antd'
import I18N from '@/I18N'
import {
  TASK_CATEGORY, TASK_TYPE, TASK_STATUS, TASK_CANDIDATE_STATUS, USER_ROLE, USER_AVATAR_DEFAULT,
} from '@/constant'
import './style.scss'
import config from '@/config'
import MediaQuery from 'react-responsive'
import _ from 'lodash'
import moment from 'moment-timezone'

export default class extends BaseComponent {
  ord_states() {
    return {
      showSendMessage: this.props.showSendMessage || false,
    }
  }

  async componentDidMount() {
    if (this.props.showUserInfo) {
      this.props.getMember(this.props.showUserInfo._id || this.props.showUserInfo.current_user_id)
    }
  }

  componentWillUnmount() {
    if (this.props.showUserInfo) {
      this.props.resetMemberDetail()
    }
  }

  getCountry(countryCode) {
    return config.data.mappingCountryCodeToName[countryCode]
  }

  sendMessage(user) {
    this.linkUserDetail(user)
  }

  linkUserDetail(user) {
    this.props.history.push(`/member/${user._id}`)
  }

  ord_render() {
    if (this.props.loading || _.isEmpty(this.props.member)) {
      return (
        <div className="flex-center spin-container">
          <Spin size="large" />
        </div>
      )
    }

    const user = this.props.member
    const avatar = user.profile.avatar || USER_AVATAR_DEFAULT
    const now = moment(Date.now())
    const localTime = user.profile.timezone
      ? now.tz(user.profile.timezone).format('LT z')
      : 'Unknown'

    return (
      <div className="c_ProfileModalPopup">
        <div className="header-image-container">
          <img src="/assets/images/city_background.png" />
        </div>
        <div>
          <div className="profile-image">
            <img src={avatar} />
          </div>
          <div>
            <div className="profile-info">
              <div className="name komu-a">
                {user.profile.firstName}
                {` ${user.profile.lastName}`}
              </div>
              <div className="location-circle">
                <span>
                  <Icon type="compass" />
                  {this.getCountry(user.profile.country)}
                </span>
                {
                  _.map(user.circles, (circle, ind) => (
                    <a
                      key={ind}
                      className="circle"
                      onClick={() => this.props.history.push(`/crcles-detail/${circle._id}`)}
                      target="_blank"
                    >
                      [
                      {circle.name}
                      {' '}
                      {I18N.get('developer.search.circle')}
                      ]
                    </a>
                  ))
                }
              </div>
            </div>
            <div className="profile-interaction">
              <div>
                <span>
                  {I18N.get('profile.localTime')}
                  {' '}
                  {localTime}
                </span>
              </div>
            </div>
            {this.state.showSendMessage ? (
              <div className="message-box">
                <UserContactForm recipient={user} close={this.props.close} />
              </div>
            ) : (
              <div className="profile-view-button">
                <Button
                  className="komu-a"
                  onClick={() => this.linkUserDetail(user)}
                >
                  {I18N.get('profile.viewProfile')}
                </Button>
              </div>
            )
            }
          </div>
        </div>
      </div>
    )
  }
}
