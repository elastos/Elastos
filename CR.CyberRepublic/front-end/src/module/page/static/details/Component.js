import React from 'react'
import './style.scss'
import { Col, Row, Icon, Button, Divider, Avatar, Spin } from 'antd'
import {
  FacebookShareButton,
  LinkedinShareButton,
  TwitterShareButton,
  TelegramShareButton,
  RedditShareButton,
  EmailShareButton,
  FacebookIcon,
  TwitterIcon,
  LinkedinIcon,
  TelegramIcon,
  RedditIcon,
  EmailIcon
} from 'react-share'
import {
  withScriptjs,
  withGoogleMap,
  GoogleMap,
  Marker
} from 'react-google-maps'
import Geocode from 'react-geocode'
import moment from 'moment'
import _ from 'lodash'
import EmptyPage from '../../EmptyPage'
import { getSafeUrl } from '@/util/url'

process.env.NODE_ENV === 'production' &&
  Geocode.setApiKey(process.env.GOOGLE_MAPS_API_KEY)

export default class extends EmptyPage {
  state = {
    // TODO: Add API key for google maps
    loading: true,
    hasLocation: false,
    lat: 18.7,
    lng: 98.98,

    defaultZoom: 10
  }

  async componentDidMount() {
    this.setState({ loading: true })
    const taskId = this.props.match.params.eventId
    await this.props.getTaskDetail(taskId)

    // we are defaulting to the country if there is no exact location
    let location

    if (this.props.task.location && this.props.task.location !== 'TBD') {
      location = this.props.task.location
      await this.setState({
        defaultZoom: 10
      })
    } else if (this.props.task.community && this.props.task.community.name) {
      location = this.props.task.community.name
      await this.setState({
        defaultZoom: 3
      })
    }

    console.log(`location: ${location} - zoom: ${this.state.defaultZoom}`)

    if (location) {
      await this.setState({
        hasLocation: true
      })
    }

    if (this.state.hasLocation) {
      Geocode.fromAddress(location).then(
        async response => {
          const { lat, lng } = response.results[0].geometry.location

          this.setState({
            lat,
            lng,
            loading: false
          })
        },
        error => {
          console.error(error)
          this.setState({ loading: false })
        }
      )
    } else {
      this.setState({ loading: false })
    }
  }

  async componentWillUnmount() {
    await this.props.resetTaskDetail()
  }

  register() {
    this.props.register(this.props.task._id, this.props.currentUserId)
  }

  deregister(candidate) {
    this.props.deregister(this.props.task._id, candidate._id)
  }

  navigateToEvents() {
    this.props.history.push('/events/')
  }

  navigateToUserProfile(userId) {
    this.props.history.push(`/member/${userId}`)
  }

  renderMapComponent() {
    const CustomMapComponent = withScriptjs(
      withGoogleMap(props => (
        <GoogleMap
          defaultZoom={this.state.defaultZoom}
          defaultCenter={{ lat: this.state.lat, lng: this.state.lng }}
        >
          {<Marker position={{ lat: this.state.lat, lng: this.state.lng }} />}
        </GoogleMap>
      ))
    )
    let url

    if (process.env.NODE_ENV === 'production') {
      url = `https://maps.googleapis.com/maps/api/js?key=${
        process.env.GOOGLE_MAPS_API_KEY
      }&v=3.exp&libraries=geometry,drawing,places`
    } else {
      url =
        'https://maps.googleapis.com/maps/api/js?v=3.exp&libraries=geometry,drawing,places'
    }
    const mapElement = (
      <CustomMapComponent
        isMarkerShown={true}
        googleMapURL={url}
        loadingElement={<div style={{ height: '100%' }} />}
        containerElement={<div style={{ height: '400px' }} />}
        mapElement={<div style={{ height: '100%' }} />}
      />
    )

    return mapElement
  }

  renderEventDetails() {
    const eventName = this.props.task.name || ''
    const hostedBy = this.props.task.createdBy
      ? `${this.props.task.createdBy.profile.firstName} ${
        this.props.task.createdBy.profile.lastName
      }`
      : 'unknown user'
    const hostedByID = this.props.task.createdBy
      ? this.props.task.createdBy._id
      : 'not-found'
    const hostedByAvatar = this.props.task.createdBy
      ? this.props.task.createdBy.profile.avatar
      : null
    const eventLocation = this.props.task.location || 'TBD'
    const eventDate = this.props.task.eventDateRangeStart
      ? moment(this.props.task.eventDateRangeStart).format(
        'MMMM Do YYYY. h:mm a'
      )
      : 'TBD'
    const eventType = this.props.task.type
      ? this.props.task.type[0] +
        this.props.task.type
          .toLowerCase()
          .substr(1, this.props.task.type.length)
      : '-'
    const eventInfo = this.props.task.infoLink || ''
    // descriptionTitle disabled until implemented backend
    const descriptionTitle = ''
    const description = this.props.task.description || ''
    const goals = this.props.task.goals || ''

    return (
      <Col sm={{ span: 24 }} md={{ span: 12 }} className="d_col_left">
        <Row type="flex">
          <Col xs={{ span: 24 }} sm={{ span: 24 }} md={{ span: 16 }}>
            <span className="event-name">{eventName}</span>
            <span className="event-hosted-by">
              Hosted by
              <a onClick={() => this.navigateToUserProfile(hostedByID)}>
                {hostedBy}
              </a>
            </span>
            <div className="event-detail-container">
              <Row>
                <Icon type="environment" className="icon-location" />
                <span className="event-location">{eventLocation}</span>
              </Row>
              <Row>
                <Icon type="clock-circle" className="icon-time" />
                <span className="event-time">{eventDate}</span>
              </Row>
              <Row>
                <Icon type="question-circle" className="icon-type" />
                <span className="event-type">{eventType}</span>
              </Row>
              <Row>
                <Icon type="info-circle" className="icon-info" />
                <span className="event-info">
                  <a target="_blank" href={getSafeUrl(eventInfo)}>
                    {eventInfo}
                  </a>
                </span>
              </Row>
            </div>
          </Col>
          <Col
            xs={{ span: 24 }}
            sm={{ span: 24 }}
            md={{ span: 8 }}
            className="hosted-by-avatar-container"
          >
            {hostedByAvatar && (
              <Avatar className="hosted-by-avatar" src={hostedByAvatar} />
            )}
          </Col>
        </Row>
        <Divider className="event-details-divider" />
        <div className="event-detail-description">
          <div className="event-detail-description-title">
            {descriptionTitle || 'Description'}
          </div>
          <div className="event-detail-description-content">{description}</div>
          {goals && (
            <div className="event-detail-goals">
              <div className="event-detail-description-title">Goals</div>
              <div>{goals}</div>
            </div>
          )}
        </div>
      </Col>
    )
  }

  renderEventActions() {
    const attendance = _.find(
      this.props.task.candidates,
      i => i.user && i.user._id === this.props.currentUserId
    )
    const eventImage =
      this.props.task.thumbnail || '/assets/images/Elastos_Logo_Temp.png'
    const shareQuote = this.props.task.name || 'Visit us at elastos.org!'

    const buttonActionLabel = attendance ? 'DEREGISTER' : 'REGISTER'
    const buttonActionClass = `actionButton ${
      attendance ? 'actionDeregister' : 'actionRegister'
    }`
    const buttonType = attendance ? 'danger' : 'primary'
    const subscriptionAction = attendance ? this.deregister : this.register

    return (
      <Col sm={{ span: 24 }} md={{ span: 12 }} className="d_col_right">
        <img src={eventImage} />
        <Button
          loading={this.props.loading}
          className={buttonActionClass}
          type={buttonType}
          onClick={subscriptionAction.bind(this, attendance)}
        >
          <span>{buttonActionLabel}</span>
        </Button>
        <span className="share-with-friends">SHARE WITH FRIENDS</span>
        <Row className="social-share-actions">
          <FacebookShareButton
            url={window.location.href}
            quote={shareQuote}
            className="share-button"
          >
            <FacebookIcon size={32} round={true} />
          </FacebookShareButton>
          <TwitterShareButton
            url={window.location.href}
            title={shareQuote}
            className="share-button"
          >
            <TwitterIcon size={32} round={true} />
          </TwitterShareButton>
          <TelegramShareButton
            url={window.location.href}
            title={shareQuote}
            className="share-button"
          >
            <TelegramIcon size={32} round={true} />
          </TelegramShareButton>
          <LinkedinShareButton
            url={window.location.href}
            title={shareQuote}
            windowWidth={750}
            windowHeight={600}
            className="share-button"
          >
            <LinkedinIcon size={32} round={true} />
          </LinkedinShareButton>
          <RedditShareButton
            url={window.location.href}
            title={shareQuote}
            windowWidth={660}
            windowHeight={460}
            className="share-button"
          >
            <RedditIcon size={32} round={true} />
          </RedditShareButton>
          <EmailShareButton
            url={window.location.href}
            subject={shareQuote}
            body="body"
            className="share-button"
          >
            <EmailIcon size={32} round={true} />
          </EmailShareButton>
        </Row>
      </Col>
    )
  }

  ord_renderContent() {
    if (this.state.loading) {
      return (
        <div className="center">
          <Spin size="large" />
        </div>
      )
    }
    const communityName =
      (this.props.task.community && this.props.task.community.name) ||
      'Elastos Event'
    const backButton = '< Back'
    // TODO: Replace d_row_lower with sensible data
    return (
      <div className="p_EVENT_DETAILS">
        <div className="ebp-page">
          <div className="ebp-events-location">{communityName}</div>
          <a onClick={() => this.navigateToEvents()}>{backButton}</a>
          <Row className="d_row_upper">
            {this.renderEventDetails()}
            {this.renderEventActions()}
          </Row>
          {this.state.hasLocation && (
            <Row className="d_row_mid">
              <div className="map">{this.renderMapComponent()}</div>
            </Row>
          )}
          {false && (
            <Row className="d_row_lower">
              <span className="title">BALAYAGE w/ KITTY COLOURIST - VIC</span>
              <span>at</span>
              <span className="title-2">La Biosthetique Academie</span>
              <span className="address">
                1209 High Street Armadale, Melbourne, Victoria 3143
              </span>
              <Row>
                <Icon type="link" />
                <Icon type="link" />
                <Icon type="link" />
                <Icon type="link" />
              </Row>
            </Row>
          )}
        </div>
      </div>
    )
  }
}
