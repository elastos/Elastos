import React from 'react'
import { Link } from 'react-router-dom'
import { DEFAULT_IMAGE, USER_GENDER } from '@/constant'
import I18N from '@/I18N'
import _ from 'lodash'

import { Card, Select, Col, Row, Button, Input } from 'antd'

import config from '@/config'
import Footer from '@/module/layout/Footer/Container'
import '../style.scss'

import ipinfo from 'ipinfo'
import StandardPage from '../../StandardPage'

const Search = Input.Search

export default class extends StandardPage {
    state = {
      communities: [],

      countryGeolocation: '',
      countryExists: true
    }

    componentWillUnmount () {
      this.props.resetTasks()
    }

    async componentDidMount() {
      await this.props.getSocialEvents()
      await this.loadCommunities()
      await this.redirectToCountry()
    }

    async loadCommunities() {
      let communities
      const currentCountry = this.props.match.params.country

      if (currentCountry) {
        communities = await this.props.getSpecificCountryCommunities(currentCountry)
      } else {
        communities = await this.props.getAllCountryCommunity()
      }

      this.setState({
        communities
      })
    }

    async redirectToCountry() {

      let countryCommunities, countryCommunity

      if (this.props.currentUser.is_login) {
        // just get the country from the user profile
        countryCommunities = await this.props.getSpecificCountryCommunities(this.props.currentUser.profile.country)
        countryCommunity = _.find(countryCommunities, {parentCommunityId: null})

        this.setState({
          countryGeolocation: countryCommunity && countryCommunity.geolocation
        })

      } else {
        // fetch the country from the backend
        const cLoc = await new Promise((resolve, reject) => {
          ipinfo((err, cLoc) => {
            if (err) {
              reject(err)
              return
            }

            resolve(cLoc)
          })
        })

        this.setState({
          countryGeolocation: cLoc.country.toLowerCase()
        })

        countryCommunities = await this.props.getSpecificCountryCommunities(this.state.countryGeolocation)
        countryCommunity = _.find(countryCommunities, {parentCommunityId: null})

      }

      if (countryCommunity) {
        // we go to that community if it exists
        this.props.history.replace(`/community/${countryCommunity._id}/country/${this.state.countryGeolocation}`)
        return
      }

      this.setState({
        countryExists: false
      })

    }

    getAvatarUrl(users) {
      const avatarDefault = {
        [USER_GENDER.MALE]: '/assets/images/User_Avatar_Other.png'
      }

      users.forEach((user) => {
        if (!user.profile.avatar) {
          user.profile.avatar = avatarDefault[USER_GENDER.MALE]
        }
      })

      return users
    }

    // API only return list leader ids [leaderIds], so we need convert it to array object leader [leaders]
    /*
    convertCommunitiesLeaderIdsToLeaderObjects(communities) {
        return new Promise((resolve, reject) => {
            let userIds = []
            communities.forEach((community) => {
                userIds.push(...community.leaderIds)
            })
            userIds = _.uniq(userIds)

            if (!userIds.length) {
                return resolve(communities)
            }

            this.props.getUserByIds(userIds).then((users) => {
                users = this.getAvatarUrl(users) // Mock avatar url
                const mappingIdToUserList = _.keyBy(users, '_id');
                communities.forEach((community) => {
                    community.leaders = community.leaders || [];
                    community.leaderIds.forEach((leaderId) => {
                        if (mappingIdToUserList[leaderId]) {
                            community.leaders.push(mappingIdToUserList[leaderId])
                        }
                    })
                })

                resolve(communities)
            })
        })
    }
    */

    getCommunityIdByGeolocation(geolocation) {
      const community = _.find(this.state.communities, {
        geolocation
      })

      if (community) {
        return community._id
      }
    }

    handleChangeCountry(geolocation) {
      if (geolocation) {
        const communityId = this.getCommunityIdByGeolocation(geolocation)
        this.props.history.push(`/community/${communityId}/country/${geolocation}`)
      } else {
        this.props.getAllCountryCommunity().then((communities) => {
          this.convertCommunitiesLeaderIdsToLeaderObjects(communities).then((communities) => {
            this.setState({
              communities
            })

            this.props.history.push('/community')
          })
        })
      }
    }

    renderBreadcrumbCountries() {
      const geolocationKeys = _.keyBy(this.state.communities, 'geolocation')
      const listCountriesEl = Object.keys(geolocationKeys).map((geolocation, index) => {
        return (
          <Select.Option title={config.data.mappingCountryCodeToName[geolocation]} key={index}
            value={geolocation}>
            {config.data.mappingCountryCodeToName[geolocation]}
          </Select.Option>
        )
      })

      return (
        <Select
          value={this.props.match.params.country || undefined}
          showSearch={true}
          style={{width: 160}}
          placeholder={I18N.get('community.selectcountry')}
          optionFilterProp="children"
          onChange={this.handleChangeCountry.bind(this)}
        >
          {listCountriesEl}
        </Select>
      )
    }

    renderListCommunities() {
      return this.state.communities.map((community, index) => {
        return (
          <div key={index}>
            {community.leaders && community.leaders.map((leader) => {
              return (
                <Col xs={{span: 12}} md={{span: 3}} key={`${index}-${leader._id}`} className="user-card public-communities-page">
                  <Link to={`/community/${community._id}/country/${community.geolocation}`}>
                    <Card
                      key={index}
                      cover={<img src={leader.profile.avatar}/>}
                    >
                      <h5>
                        {community.name}
                      </h5>
                      {/* // TODO: we want a better way to show multiple organizers
                                        <p className="user-info">
                                            {leader.profile.firstName + ' ' + leader.profile.lastName}<br/>
                                            <span class="no-info">{leader.username}</span>
                                        </p>
                                        */}
                    </Card>
                  </Link>
                </Col>
              )
            })}

            {(!community.leaders || community.leaders.length === 0) && (
              <Col xs={{span: 12}} md={{span: 3}} key={index} className="user-card public-communities-page">
                <Link to={`/community/${community._id}/country/${community.geolocation}`}>
                  <Card
                    key={index}
                    cover={<img src={DEFAULT_IMAGE.UNSET_LEADER}/>}
                  >
                    <h5>
                      {community.name}
                    </h5>
                  </Card>
                </Link>
              </Col>
            )}
          </div>
        )
      })
    }

    ord_renderContent () {
      // const listCommunitiesEl = this.renderListCommunities()
      const menuCountriesEl = this.renderBreadcrumbCountries()

      return (
        <div className="p_Community">
          <div className="ebp-header-divider" />
          <div className="ebp-page">
            <div className="ebp-page">
              <div className="ebp-page-content">
                <Row>
                  <Col span={24}
                    className="community-left-column">
                    {!this.state.countryExists && (
                    <div className="guide-container">
                      <h4>
                        {I18N.get('community.guidecontainer.part1')}
                                            &nbsp;
                        {config.data.mappingCountryCodeToName[this.state.countryGeolocation]}
                      </h4>

                      <p>
                        {I18N.get('community.guidecontainer.part2')}
                      </p>
                      <p>
                        {I18N.get('community.guidecontainer.part3')}
                      </p>

                      <br/>

                      <Button onClick={() => this.props.history.push('/register')}>{I18N.get('community.button.register')}</Button>

                      <p>
                        <br/>
                        <span className="no-info">{I18N.get('community.button.selectcountry')}</span>
                      </p>
                    </div>
                    )}
                  </Col>
                </Row>
              </div>
            </div>
          </div>
          <Footer />
        </div>
      )
    }
}
