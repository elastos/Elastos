import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import moment from 'moment'
import {
  Col,
  Row,
  Icon,
  Carousel,
  Button,
  Spin,
  Form,
  Modal,
  Popover
} from 'antd'
import I18N from '@/I18N'
import {TASK_CANDIDATE_STATUS, USER_AVATAR_DEFAULT} from '@/constant'
import ProfilePopup from '@/module/profile/OverviewPopup/Container'
import _ from 'lodash'
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
    const taskId = this.props.taskId
    this.props.getTaskDetail(taskId)
  }

  componentWillUnmount() {
    this.props.resetTaskDetail()
  }

  // Renderers
  ord_render() {
    const detail = this.props.detail
    const loading = this.props.loading || _.isEmpty(this.props.detail)

    return (
      <div className="c_TaskPopup">
        {loading
          ? (
            <div className="full-width full-height valign-wrapper halign-wrapper">
              <Spin className="loading-spinner"/>
            </div>
          )
          : (
            <div>
              {this.renderHeader()}
              {this.getCarousel()}
              {this.renderMeta()}
              {this.renderFooter()}
            </div>
          )
        }
        <Modal
          className="profile-overview-popup-modal"
          visible={!!this.state.showUserInfo}
          onCancel={this.handleCancelProfilePopup.bind(this)}
          footer={null}>
          {this.state.showUserInfo &&
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
          <div className="ql-editor" dangerouslySetInnerHTML={{__html: sanitizeHtml(value)}}/>
        </Col>
      </Row>
    )

    const detail = this.props.detail
    const budget = this.getBudgetFormatted()
    const reward = this.getRewardFormatted()
    const EVENT_DATE_FORMAT = 'MMM D, YYYY - HH:mm'
    const DEADLINE_FORMAT = 'MMM D'

    return (
      <div className="app-meta">
        {generateRow(I18N.get('task.owner'), (
          <a onClick={this.linkProfileInfo.bind(this, detail.createdBy)}>
            {this.getUserNameWithFallback(detail.createdBy)}
          </a>
        ))}

        {detail.circle &&
        generateRow(I18N.get('task.circle'), detail.circle.name)}

        {generateRow(I18N.get('task.type'), detail.type)}

        {generateRow(I18N.get('task.category'), detail.category)}

        {detail.location && generateRow(I18N.get('task.location'), detail.location)}

        {detail.community && generateRow(I18N.get('task.community'), this.getCommunityDisp())}

        {detail.applicationDeadline &&
        generateRow(I18N.get('task.applyDeadline'),
          moment(detail.applicationDeadline).format(DEADLINE_FORMAT))}

        {detail.completionDeadline &&
        generateRow(I18N.get('task.completionDeadline'),
          moment(detail.completionDeadline).format(DEADLINE_FORMAT))}

        {detail.bidding &&
        generateRow(I18N.get('task.referenceBid'),
          detail.referenceBid
            ? `${detail.referenceBid} ELA`
            : I18N.get('task.referenceBid.none'))}

        {!detail.bidding && budget && generateRow(I18N.get('task.budget'), (
          <div>
            <span>{budget}</span>
            {this.getBudgetExplanation()}
          </div>
        )) || null}

        {!detail.bidding && reward && generateRow(I18N.get('task.reward'), (
          <div>
            <span>{reward}</span>
            {this.getRewardExplanation()}
          </div>
        )) || null}

        {detail.status && generateRow(I18N.get('profile.tasks.table.status'), `${detail.status} ${_.find(detail.candidates, {status: TASK_CANDIDATE_STATUS.APPROVED}) ? '- (assigned)' : ''}`)}

        {detail.goals && generateRow(I18N.get('task.goals'), detail.goals, 'task-goals')}

        {detail.descBreakdown && generateHtmlRow(I18N.get('task.descBreakdown'),
          detail.descBreakdown, 'task-breakdown')}

        {detail.eventDateRangeStart && generateRow(I18N.get('task.eventStart'),
          `${moment(detail.eventDateRangeStart).format(EVENT_DATE_FORMAT)} (${
            detail.eventDateStatus})`)}

        {detail.eventDateRangeEnd && generateRow(I18N.get('task.eventEnd'),
          moment(detail.eventDateRangeEnd).format(EVENT_DATE_FORMAT))}

        {generateHtmlRow(I18N.get('task.description'), detail.description, 'task-description')}

        {detail.infoLink && generateRow(I18N.get('task.infoLink'),
          <a href={getSafeUrl(detail.infoLink)} target="_blank">{detail.infoLink}</a>)}
      </div>
    )
  }

  renderFooter() {
    const detailUrl = `/task-detail/${this.props.detail._id}`
    return (
      <div className="app-footer valign-wrapper halign-wrapper">
        <Button href={getSafeUrl(detailUrl)}>
          {this.isAssigned() ? I18N.get('project.detail.view') : I18N.get('task.applyMessage')}
        </Button>
      </div>
    )
  }

  // Helpers
  getCurrency() {
    return 'USD'
  }

  getReward() {
    return this.props.detail.reward && (this.props.detail.reward.usd / 100)
  }

  getRewardElaPerUsd() {
    return this.props.detail.reward && this.props.detail.reward.elaPerUsd
  }

  getRewardFormatted() {
    const epu = this.getRewardElaPerUsd()
    const suffix = epu ? ` (@${epu} ELA/USD)` : ''
    return this.getReward() && `${this.getReward()} ${this.getCurrency()}${suffix}`
  }

  getBudget() {
    return this.props.detail.rewardUpfront && (this.props.detail.rewardUpfront.usd / 100)
  }

  getBudgetElaPerUsd() {
    return this.props.detail.rewardUpfront && this.props.detail.rewardUpfront.elaPerUsd
  }

  getBudgetFormatted() {
    const epu = this.getBudgetElaPerUsd()
    const suffix = epu ? ` (@${epu} ELA/USD)` : ''
    return this.getBudget() && `${this.getBudget()} ${this.getCurrency()}${suffix}`
  }

  getCommunityDisp() {
    let str = ''

    if (this.props.detail.communityParent) {
      str += `${this.props.detail.communityParent.name}/`
    }
    if (this.props.detail.community) {
      str += this.props.detail.community.name
    }

    return str
  }

  getBudgetExplanation() {
    return (
      <Popover content={I18N.get('task.budget.explain')}>
        <Icon className="help-icon" type="question-circle-o"/>
      </Popover>
    )
  }

  getRewardExplanation() {
    return (
      <Popover content={I18N.get('task.reward.explain')}>
        <Icon className="help-icon" type="question-circle-o"/>
      </Popover>
    )
  }

  isTaskOwner() {
    return this.props.detail.createdBy._id === this.props.currentUserId
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
               alt="logo" src={picture.url}/>
        </div>
      )
    })

    if (item.thumbnail) {
      pictures.unshift(
        <div key="main">
          <img width={IMAGE_SIZE} height={IMAGE_SIZE}
               alt="logo" src={item.thumbnail}/>
        </div>
      )
    }

    if (pictures.length === 0) {
      pictures.push(<img width={IMAGE_SIZE} height={IMAGE_SIZE}
                         src="/assets/images/Group_1685.12.svg" key={0}/>)
    }

    return (
      <div className="carousel-wrapper">
        <Carousel autoplay={true}>
          {pictures}
        </Carousel>
      </div>
    )
  }

  showTaskDetail() {
    this.props.history.push(`/task-detail/${this.props.detail._id}`)
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

  isAssigned() {
    return !!_.find(this.props.detail.candidates, {status: TASK_CANDIDATE_STATUS.APPROVED})
  }

  handleCancelProfilePopup() {
    this.setState({
      showUserInfo: null
    })
  }
}

export default Form.create()(C)
