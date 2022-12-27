import React from 'react'
import MediaQuery from 'react-responsive'
import './style.scss'
import {Row, Icon, Button, Checkbox, Card, Tag, Cascader, Select, Divider} from 'antd'
import moment from 'moment/moment'
import _ from 'lodash'
import {MAX_WIDTH_MOBILE, MIN_WIDTH_PC} from '../../../../config/constant'
import EmptyPage from '../../EmptyPage'

const Option = Select.Option

export default class extends EmptyPage {

    state = {
      activeMonth: new Date().getMonth(),
      communityTrees: [],
      filterCommunity: [],
      favorites: [],
      showFavoritesOnly: false
    };

    async componentDidMount() {
      this.props.getSocialEvents()
      this.getAllCommunities()
    }

    getAllCommunities() {
      this.props.getAllCommunities().then((communityTrees) => {
        this.setState({
          communityTrees
        })
      })
    }

    navigateToEvent(socialEventID) {
      this.props.history.push(`/events/${socialEventID}`)
    }

    handleMonthChange(month) {
      this.setState({
        activeMonth: month
      })
    }

    renderMonthsAsBar() {
      const months = moment.months()
      const monthsElements = []

      for (let i = 0; i < months.length; i++) {
        const idx = (i === this.state.activeMonth && months[i].length === 4) ? 4 : 3
        const monthClass = i === this.state.activeMonth ? 'ebp-events-month ebp-events-active-month' : 'ebp-events-month'
        monthsElements.push(
          <span className={monthClass} key={i} onClick={this.handleMonthChange.bind(this, i)}>
            {months[i].substr(0, idx)}
          </span>
        )
      }
      return monthsElements
    }

    renderMonthsAsDropDown() {
      const months = moment.months()

      const options = []
      for (let i = 0; i < months.length; i++) {
        options.push(<Option value={i} key={i}>{months[i]}</Option>)
      }

      return (
        <Select
          showSearch={true}
          allowClear={true}
          className="select-months"
          defaultValue={this.state.activeMonth}
          placeholder="Filter by month"
          optionFilterProp="children"
          onChange={(e) => this.handleMonthChange(e)}
          filterOption={(input, option) => option.props.children.toLowerCase().indexOf(input.toLowerCase()) >= 0}>
          {options}
        </Select>
      )
    }

    renderCommunityDropDown() {
      return (
        <Cascader className="community-filter"
        value={[...this.state.filterCommunity]}
        style={{width: '250px'}}
        options={this.state.communityTrees}
        placeholder="Filter by community"
        onChange={this.handleOnChangeFilter.bind(this)}
        changeOnSelect={true} />
      )
    }

    onFavoriteFilterChange(value) {
      this.setState({
        showFavoritesOnly: value.target.checked
      })
    }

    renderFavoritesFilter() {
      return (
        <Checkbox className="favorites-filter" onChange={(e) => this.onFavoriteFilterChange(e)}>
                Show only favorites (
          <Icon className="star" type="star" />
)
        </Checkbox>
      )
    }

    handleOnChangeFilter(value) {
      this.setState({
        filterCommunity: value
      })
    }

    handleRegisterUser(socialEventId) {
      this.props.register(socialEventId, this.props.currentUserId)
    }

    handleUnregisterUser(candidate, socialEventId) {
      this.props.deregister(socialEventId, candidate._id)
    }

    animateStar(socialEventId) {
      const favorites = this.state.favorites
      const found = favorites.find((item) => item.key === socialEventId)

      if (found) {
        const idx = favorites.indexOf(found)
        favorites[idx].value = !favorites[idx].value
      } else {
        favorites.push({
          key: socialEventId,
          value: true
        })
      }

      this.setState({
        favorites
      })
    }

    getStarActiveClass() {
      return 'star-poly-active'
    }

    renderSubscriptionButton(candidate, socialEventId) {
      const register = (
        <Button className="events-card-button-register" type="primary" loading={this.props.loading}
          onClick={() => this.handleRegisterUser(socialEventId)}>
          <span>Register</span>
        </Button>
      )
      const unregister = (
        <Button className="events-card-button-unregister" loading={this.props.loading}
          onClick={() => this.handleUnregisterUser(candidate, socialEventId)}>
          <span className="going-text">Going</span>
          <span className="cancel-text">Cancel?</span>
        </Button>
      )

      if (!candidate) {
        return register
      }
      return unregister
    }

    renderDetails(socialEventId) {
      return (
        <div className="events-card-details-button">
          <Button onClick={() => this.navigateToEvent(socialEventId)}>Find out more</Button>
        </div>
      )
    }

    // Disabled until hash-tags implemented backend
    renderHashTags(hashTags) {
      const elements = []
      for (let i = 0; i < hashTags.length; i++) {
        elements.push(<Tag key={i}>{hashTags[i]}</Tag>)
      }
      return elements
    }

    renderEventCard(socialEvent) {
      const candidate = socialEvent.candidates
        ? socialEvent.candidates.find((user) => user.user && user.user._id === this.props.currentUserId) : null
      const hashTags = ['#4ever', '#elastos']
      const image = socialEvent.thumbnail ? socialEvent.thumbnail
        : '/assets/images/Elastos_Logo_Temp.png'
      const date = socialEvent.eventDateRangeStart ? moment(socialEvent.eventDateRangeStart).format('MMMM Do YYYY - h:mma') : 'TBD'
      const community = socialEvent.location || 'Location TBD'
      const svgStarClass = this.state.favorites.find((item) => item.key === socialEvent._id && item.value) ? this.getStarActiveClass() : 'star-poly'

      return (
        <Card
          key={socialEvent._id}
          cover={<img className="event-card-image" src={image} />}>
          <div className="card-icon" onClick={() => this.navigateToEvent(socialEvent._id)} style={{cursor: 'pointer'}}/>
          {/* <div className="card-icon" onClick={() => this.animateStar(socialEvent._id)}>
                    <svg width="50%" height="50%" viewBox="0 0 40 37" className="star">
                        <polygon className={svgStarClass} points="272 30 260.244 36.18 262.489 23.09 252.979 13.82 266.122 11.91 272 0 277.878 11.91 291.021 13.82 281.511 23.09 283.756 36.18" transform="translate(-252)"/>
                    </svg>
                </div> */}
          <div className="events-card-detail">
            <div className="events-card-time">{date}</div>
            <div className="events-card-title"
              onClick={() => this.navigateToEvent(socialEvent._id)}>
              {socialEvent.name}
            </div>
            <div className="events-card-location">{community}</div>
            <div className="events-card-button-container">
              {this.renderSubscriptionButton(candidate, socialEvent._id)}
            </div>
            <div className="events-card-details">
              {this.renderDetails(socialEvent._id)}
            </div>
          </div>
          {false && (
          <div className="events-card-hashtags">
                        this.renderHashTags(hashTags)
          </div>
          )}
        </Card>
      )
    }

    getFilteredEvents(socialEvents) {
      socialEvents = _.filter(socialEvents, (item) => {

        if (this.state.showFavoritesOnly &&
                !this.state.favorites.find((favorite) => favorite.key === item._id && favorite.value)) {
          return false
        }

        // event must have a date to be shown
        const dateValid = item.eventDateRangeStart
          ? new Date(item.eventDateRangeStart).getMonth() === this.state.activeMonth : false

        if (this.state.filterCommunity.length === 0) {
          return dateValid
        }
        if (!item.community) {
          return dateValid && this.state.filterCommunity.length === 0
        }

        let communityValid = false
        if (this.state.filterCommunity.length > 1) {
          if (item.community._id === this.state.filterCommunity[1]) {
            communityValid = true
            return dateValid && communityValid
          }
          return false
        } if (this.state.filterCommunity[0] === item.community._id) {
          communityValid = true
        } else {
          const found = this.state.communityTrees.find((community) => community.value === this.state.filterCommunity[0])
          if (found && found.children) {
            for (let j = 0; j < found.children.length; j++) {
              if (found.children[j].value === item.community._id) {
                communityValid = true
                return dateValid && communityValid
              }
            }
          }
        }
        return dateValid && communityValid
      })

      socialEvents.sort((a, b) => {
        return moment(a.eventDateRangeStart) - moment(b.eventDateRangeStart)
      })

      return socialEvents
    }

    renderEventCards(socialEvents) {
      const eventCards = []
      const filteredSocialEvents = this.getFilteredEvents(socialEvents)
      for (let i = 0; i < filteredSocialEvents.length; i++) {
        eventCards.push(this.renderEventCard(filteredSocialEvents[i]))
      }
      return eventCards
    }

    ord_renderContent () {
      return (
        <div className="p_EVENTS">
          <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
            <div className="mobile-header">
              <span className="mobile-header-title">Events</span>
            </div>
          </MediaQuery>
          <div className="ebp-page">
            <div className="ebp-events-time">
              <MediaQuery minWidth={MIN_WIDTH_PC}>
                <Row type="flex" justify="center" className="ebp-months-bar">
                  {this.renderMonthsAsBar()}
                </Row>
              </MediaQuery>
              <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
                {this.renderMonthsAsDropDown()}
              </MediaQuery>
            </div>
            <div className="ebp-events-location">
              {this.renderCommunityDropDown()}
            </div>
            {/* <div className="ebp-events-favorites">
                        {this.renderFavoritesFilter()}
                    </div> */}
            <Divider />
            <div className="events-count">
              {this.getFilteredEvents(this.props.all_tasks).length}
              {' '}
events total
            </div>
            <Row className="d_row" type="flex" justify="left">
              {this.renderEventCards(this.props.all_tasks)}
            </Row>
          </div>
        </div>
      )
    }
}
