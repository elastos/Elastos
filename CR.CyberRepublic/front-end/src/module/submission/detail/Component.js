import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import moment from 'moment'
import Comments from '@/module/common/comments/Container'

import {Col, Row, Divider, Icon, Tag, Spin, Avatar} from 'antd'

import {SUBMISSION_TYPE, SUBMISSION_CAMPAIGN} from '@/constant'
import _ from 'lodash'
import { getSafeUrl } from '@/util/url'

import '../style.scss'
import './style.scss'

export default class extends BaseComponent {

  constructor(props) {
    super(props)

    this.state = {
      attachment_url: this.props.submission.attachment,
      attachment_loading: false,
      attachment_filename: this.props.submission.attachmentFilename,
      attachment_type: this.props.submission.attachmentType,

      removeAttachment: false
    }
  }

  // special layout for external forms
  renderFormExt() {
    return (
      <div>
        <Row>
          <Col>
            <h4 className="center">
              {this.props.submission.title}
            </h4>
          </Col>
        </Row>
        <Row>
          <Col className="col-label" span={4}>
          Email
          </Col>
          <Col span={16}>
            {this.props.submission.email}
          </Col>
        </Row>
        <Row>
          <Col className="col-label" span={4}>
          Full Legal Name
          </Col>
          <Col span={16}>
            {this.props.submission.fullLegalName}
          </Col>
        </Row>
        {this.props.submission.occupation && (
        <Row>
          <Col className="col-label" span={4}>
          Occupation
          </Col>
          <Col span={16}>
            {this.props.submission.occupation}
          </Col>
        </Row>
        )}
        {this.props.submission.education && (
        <Row>
          <Col className="col-label" span={4}>
          Education
          </Col>
          <Col span={16}>
            {this.props.submission.education}
          </Col>
        </Row>
        )}
        <Row>
          <Col className="form-detail-desc" offset={4} span={16}>
          What is your native language, who is your audience and where are they located? What are the language(s) you
          plan to use to present Elastos.
          </Col>
        </Row>
        <Row>
          <Col offset={4} span={16}>
            {this.props.submission.audienceInfo}
          </Col>
        </Row>
        <Row>
          <Col className="form-detail-desc" offset={4} span={16}>
          Please describe your public speaking experience and provide any examples.
          </Col>
        </Row>
        <Row>
          <Col offset={4} span={16}>
            {this.props.submission.publicSpeakingExp}
          </Col>
        </Row>
        {this.props.submission.eventOrganizingExp && (
        <div>
          <Row>
            <Col className="form-detail-desc" offset={4} span={16}>
            Do you have any experience organizing events and provide any examples.
            </Col>
          </Row>
          <Row>
            <Col offset={4} span={16}>
              {this.props.submission.eventOrganizingExp}
            </Col>
          </Row>
        </div>
        )}
        <Row>
          <Col className="form-detail-desc" offset={4} span={16}>
          Please list any current or past contributions promoting Elastos.
          </Col>
        </Row>
        <Row>
          <Col offset={4} span={16}>
            {this.props.submission.previousExp}
          </Col>
        </Row>
        <Divider />
        <Row>
          <Col className="col-label" span={4}>
          Are you a developer?
          </Col>
          <Col span={16}>
            {this.props.submission.isDeveloper ? 'Yes' : 'No'}
          </Col>
        </Row>
        {!this.props.submission.isDeveloper && (
        <div>
          <Row>
            <Col className="form-detail-desc" offset={4} span={16}>
            If you are not a developer, please explain how you are familiar with Elastos technology and what problems we
            solve.
            </Col>
          </Row>
          <Row>
            <Col offset={4} span={16}>
              {this.props.submission.devBackground}
            </Col>
          </Row>
        </div>
        )}
        <Row>
          <Col className="form-detail-desc" offset={4} span={16}>
          Describe Elastos in your own words.
          </Col>
        </Row>
        <Row>
          <Col offset={4} span={16}>
            {this.props.submission.description}
          </Col>
        </Row>
        <Row>
          <Col className="form-detail-desc" offset={4} span={16}>
          Tell us in a few words what inspired you to join Cyber Republic.
          </Col>
        </Row>
        <Row>
          <Col offset={4} span={16}>
            {this.props.submission.reason}
          </Col>
        </Row>
        <Row>
          <Col className="form-detail-desc" offset={4} span={16}>
          Please submit a video of your introduction to Cyber Republic.
          </Col>
        </Row>
        <Row>
          <Col offset={4} span={16}>
            <a target="_blank" href={getSafeUrl(this.props.submission.attachment)}>
              <Icon type="file"/>
              {' '}
&nbsp;
              {this.props.submission.attachmentFilename}
            </a>
          </Col>
        </Row>
      </div>
    )
  }

  getUpperLeftBox() {
    const submission = this.props.submission
    return (
      <div className="left-container">
        <div className="pictures-container">
          <Icon type="customer-service"/>
        </div>
        <div className="domains-container">
          <Tag key={submission.type}>{submission.type}</Tag>
        </div>
      </div>
    )
  }

  getUpperRightBox() {
    const {description, title, createdBy, createdAt, community} = this.props.submission
    const name = community ? community.name : null
    const type = community ? community.type : null
    const profile = createdBy && createdBy.profile ? createdBy.profile : null
    const leaderImage = profile ? profile.avatar : ''
    const leaderName = createdBy.profile ? (`${profile.firstName} ${profile.lastName}`) : ''
    const createdDate = moment(createdAt).format('MMM D, YYYY')

    return (
      <div>
        <div className="title">
          <span>{title || ''}</span>
        </div>
        <a className="leader" onClick={this.linkUserDetail.bind(this, createdBy)}>
          <Avatar size="large" src={leaderImage}/>
          <div className="ellipsis">{leaderName}</div>
        </a>
        <div className="content">
          {name && type && (
          <div className="entry">
Community:
            {`${name} ${type}`}
          </div>
          )}
          <div className="entry">
Created Date:
            {createdDate}
          </div>
        </div>
        <div className="description-box">
          <hr className="divider"/>
          <div className="description-title">
            {this.props.submission.campaign === 'Evangelist Training 1' ?
              'Describe Elastos in your own words.' :
              'Description'
            }
          </div>
          <hr className="divider"/>
          <div className="description-content">{description || ''}</div>
        </div>
      </div>
    )
  }

  renderDetail() {
    const loading = _.isEmpty(this.props.submission)
    return (
      <div className="c_SubmissionDetail">
        {loading
          ? (
            <div className="full-width full-height valign-wrapper halign-wrapper">
              <Spin className="loading-spinner"/>
            </div>
          )
          : (
            <div>
              <Row className="top-section">
                <Col xs={24} sm={24} md={8} className="col-left">
                  {this.getUpperLeftBox()}
                </Col>

                <Col xs={24} sm={24} md={16} className="col-right">
                  {this.getUpperRightBox()}
                </Col>
              </Row>

              <Row>
                <Comments type="submission" canPost={true} model={this.props.submission}
                          canSubscribe={this.canSubscribe()}
                          returnUrl={`/submission-detail/${this.props.submission._id}`}
                />
              </Row>
            </div>
          )
        }
      </div>
    )
  }

  canSubscribe() {
    return !this.props.submission.createdBy ||
      (this.props.submission.createdBy &&
        this.props.submission.createdBy._id !== this.props.currentUserId)
  }

  ord_render() {
    return (
      <div className="public">
        {this.renderDynamic()}
      </div>
    )
  }

  renderDynamic() {
    if (this.props.submission.type === SUBMISSION_TYPE.FORM_EXT) {
      /*
      switch (this.props.submission.campaign) {

        case SUBMISSION_CAMPAIGN.ANNI_2008:
          return detailAnni2008.call(this)

        case SUBMISSION_CAMPAIGN.ANNI_VIDEO_2008:
          return detailAnniVideo2008.call(this)
      }
      */
    }

    return this.renderDetail()
  }

  linkUserDetail(user) {
    this.props.history.push(`/member/${user._id}`)
  }
}
