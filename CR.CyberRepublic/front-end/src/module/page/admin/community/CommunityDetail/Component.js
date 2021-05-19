import React from 'react'
import _ from 'lodash'
import AdminPage from '../../BaseAdmin'
import { Button, Card, Col, Select, Row, Icon, message, Divider, Breadcrumb } from 'antd'
import ModalChangeOrganizerCountry from '../../shared/ModalChangeOrganizerCountry/Component'
import ModalAddSubCommunity from '../../shared/ModalAddSubCommunity/Component'
import ModalUpdateSubCommunity from '../../shared/ModalUpdateSubCommunity/Component'
import ModalAddOrganizer from '../../shared/ModalAddOrganizer/Component'
import Navigator from '../../../shared/HomeNavigator/Container'
import config from '@/config'
import { COMMUNITY_TYPE, USER_GENDER, DEFAULT_IMAGE } from '@/constant'
import { logger } from '@/util'
import '../style.scss'

export default class extends AdminPage {
    state = {
      visibleModalChangeOrganizer: false,
      visibleModalAddSubCommunity: false,
      visibleModalUpdateSubCommunity: false,
      visibleModalAddOrganizer: false,
      communityType: null,
      listSubCommunitiesByType: null,
      showAllSubCommunity: {
        [COMMUNITY_TYPE.STATE]: false,
        [COMMUNITY_TYPE.CITY]: false,
        [COMMUNITY_TYPE.REGION]: false,
        [COMMUNITY_TYPE.SCHOOL]: false,
      },
      showMoreMinimum: 4,
      breadcrumbRegions: [],
      community: null,
      editedSubCommunity: null,
      editedOrganizer: null,
      users: [],
      communities: []
    }

    // Modal add organizer
    showModalAddOrganizer = () => {
      this.formRefAddOrganizer.props.form.setFieldsValue({
        geolocation: this.props.match.params.country,
      }, () => {
        this.setState({
          visibleModalAddOrganizer: true,
        })
      })
    }

    handleCancelModalAddOrganizer = () => {
      const form = this.formRefAddOrganizer.props.form
      form.resetFields()

      this.setState({visibleModalAddOrganizer: false})
    }

    handleCreateOrganizer = () => {
      const form = this.formRefAddOrganizer.props.form

      form.validateFields((err, values) => {
        if (err) {
          return
        }

        form.resetFields()
        this.setState({visibleModalAddOrganizer: false})

        const leaderIds = this.state.community.leaderIds ? [...this.state.community.leaderIds] : []
        leaderIds.push(values.leader)
        const communityClone = {
          ...this.state.community,
          leaderIds
        }

        this.props.updateCommunity(communityClone).then(() => {
          message.success('Add new organizer successfully')
          this.loadCommunityDetail()
        }).catch((err) => {
          message.error('Error while add organizer')
          logger.error(err)
        })
      })
    }

    saveFormAddOrganizerRef = (formRef) => {
      this.formRefAddOrganizer = formRef
    }

    componentDidMount() {
      super.componentDidMount()
      this.loadCommunities()
      this.loadCommunityDetail()
      this.loadSubCommunities()
      this.props.getAllUsers().then((result) => {
        this.setState({
          users: result.list
        })
      })
    }

    loadCommunities() {
      this.props.getAllCountryCommunity().then((communities) => {
        this.convertCommunitiesLeaderIdsToLeaderObjects(communities).then((communities) => {
          this.setState({
            communities
          })
        })
      })
    }

    loadCommunityDetail(id) {
      this.props.getCommunityDetail(id || this.props.match.params.community).then((community) => {
        this.convertCommunityLeaderIdsToLeaderObjects(community).then((community) => {
          this.setState({
            community
          })
        })
      })
    }

    getAvatarUrl(users) {
      const avatarDefault = {
        [USER_GENDER.MALE]: '/assets/images/User_Avatar_Other.png',
      }

      users.forEach((user) => {
        if (!user.profile.avatar) {
          user.profile.avatar = avatarDefault[USER_GENDER.MALE]
        }
      })

      return users
    }

    // API only return list leader ids [leaderIds], so we need convert it to array object leader [leaders]
    convertCommunityLeaderIdsToLeaderObjects(community) {
      return new Promise((resolve, reject) => {
        const userIds = _.uniq(community.leaderIds)
        if (!userIds.length) {
          return resolve(community)
        }
        this.props.getUserByIds(userIds).then((users) => {
          users = this.getAvatarUrl(users) // Mock avatar url
          const mappingIdToUserList = _.keyBy(users, '_id')
          community.leaders = community.leaders || []
          community.leaderIds.forEach((leaderId) => {
            if (mappingIdToUserList[leaderId]) {
              community.leaders.push(mappingIdToUserList[leaderId])
            }
          })

          resolve(community)
        })
      })
    }

    convertCommunitiesLeaderIdsToLeaderObjects(communities) {
      return new Promise((resolve, reject) => {
        let userIds = []
        communities.forEach((community) => {
          userIds.push(...community.leaderIds)
        })
        userIds = _.uniq(userIds)

        if (!userIds.length) {
          return resolve([])
        }

        this.props.getUserByIds(userIds).then((users) => {
          users = this.getAvatarUrl(users) // Mock avatar url
          const mappingIdToUserList = _.keyBy(users, '_id')
          communities.forEach((community) => {
            community.leaders = community.leaders || []
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

    loadSubCommunities(parentCommunityId) {
      this.props.getSubCommunities(parentCommunityId || this.props.match.params.community).then((subCommunities) => {
        this.convertCommunitiesLeaderIdsToLeaderObjects(subCommunities).then((subCommunities) => {
          // Check which communities we will use to render
          const listSubCommunitiesByType = this.getListSubCommunitiesByType(subCommunities, this.props.match.params.region)
          const breadcrumbRegions = this.getBreadcrumbRegions(subCommunities)

          // Update to state
          this.setState({
            subCommunities,
            listSubCommunitiesByType,
            breadcrumbRegions,
          })
        })
      })
    }

    getBreadcrumbRegions(subCommunities) {
      // Filter communities to get breadcrumb regions
      let breadcrumbRegions = []
      subCommunities.forEach((community) => {
        breadcrumbRegions.push({
          name: community.name,
        })
      })

      breadcrumbRegions = _.sortBy(_.uniqBy(breadcrumbRegions, 'name'), 'name')

      return breadcrumbRegions
    }

    // Modal change organizer
    showModalChangeOrganizer = (editedOrganizer) => {
      this.setState({
        editedOrganizer,
        visibleModalChangeOrganizer: true,
      })
    }

    handleCancelModalChangeOrganizer = () => {
      const form = this.formRefChangeOrganizer.props.form
      form.resetFields()

      this.setState({visibleModalChangeOrganizer: false})
    }

    handleChangeOrganizerCountry = () => {
      const form = this.formRefChangeOrganizer.props.form
      form.validateFields((err, values) => {
        if (err) {
          return
        }

        const leaderIds = this.state.community.leaderIds.filter((leaderId) => {
          return leaderId !== this.state.editedOrganizer
        })

        leaderIds.push(values.leader)

        this.props.updateCommunity({
          ...this.state.community,
          leaderIds,
        }).then(() => {
          form.resetFields()
          this.setState({visibleModalChangeOrganizer: false})
          message.success('Change organizer successfully')

          this.loadCommunityDetail()
        })
      })
    }

    saveFormChangeOrganizerRef = (formRef) => {
      this.formRefChangeOrganizer = formRef
    }

    openChangeOrganizerCountry (leader) {
      this.formRefChangeOrganizer.props.form.setFieldsValue({
        geolocation: this.props.match.params.country,
        leader: leader._id,
      }, this.showModalChangeOrganizer(leader._id))
    }

    handleRemoveCountry = () => {
      alert('TODO confirm spec when click button Remove country')
    }

    // Modal add community
    showModalAddSubCommunity = (type) => {
      this.formRefAddSubCommunity.props.form.setFieldsValue({
        country: this.props.match.params.country,
      }, () => {
        this.setState({
          visibleModalAddSubCommunity: true,
          communityType: type
        })
      })
    }

    handleCancelModalAddSubCommunity = () => {
      const form = this.formRefAddSubCommunity.props.form
      form.resetFields()

      this.setState({visibleModalAddSubCommunity: false})
    }

    handleAddSubCommunity = () => {
      const form = this.formRefAddSubCommunity.props.form

      form.validateFields((err, values) => {
        if (err) {
          return
        }

        this.proxyCreateSubCommunity(values)
        form.resetFields()
        this.setState({visibleModalAddSubCommunity: false})
      })
    }

    saveFormAddSubCommunityRef = (formRef) => {
      this.formRefAddSubCommunity = formRef
    }

    // Modal update community
    showModalUpdateSubCommunity = (community, leader) => {
      this.setState({
        editedSubCommunity: community
      })

      this.formRefUpdateSubCommunity.props.form.setFieldsValue({
        country: this.props.match.params.country,
        name: community.name,
        leader: leader ? leader._id : null,
      }, () => {
        this.setState({
          visibleModalUpdateSubCommunity: true,
          communityType: community.type
        })
      })
    }

    handleCancelModalUpdateSubCommunity = () => {
      const form = this.formRefUpdateSubCommunity.props.form
      form.resetFields()

      this.setState({visibleModalUpdateSubCommunity: false})
    }

    handleUpdateSubCommunity = () => {
      const form = this.formRefUpdateSubCommunity.props.form

      form.validateFields((err, values) => {
        if (err) {
          return
        }

        this.props.updateCommunity({
          ...this.state.editedSubCommunity,
          name: values.name,
          leaderIds: values.leader
        }).then(() => {
          form.resetFields()
          this.setState({visibleModalUpdateSubCommunity: false})

          message.success('Update community successfully')

          this.loadSubCommunities()
        })
      })
    }

    handleDeleteSubCommunity = () => {
      this.props.deleteCommunity(this.state.editedSubCommunity._id).then(() => {
        this.setState({visibleModalUpdateSubCommunity: false})
        message.success('Delete community successfully')
        this.loadSubCommunities()
      })
    }

    saveFormUpdateSubCommunityRef = (formRef) => {
      this.formRefUpdateSubCommunity = formRef
    }

    handleShowAllSubCommunity(type) {
      const showAllSubCommunity = this.state.showAllSubCommunity
      showAllSubCommunity[type] = !showAllSubCommunity[type]
      this.setState({
        showAllSubCommunity
      })
    }

    proxyCreateSubCommunity(formValues) {
      this.props.createSubCommunity({
        parentCommunityId: this.props.match.params.community,
        type: this.state.communityType,
        leaderIds: formValues.leader,
        // TODO check correct value of geolocation
        geolocation: this.props.match.params.country,
        name: formValues.name,
      }).then(() => {
        message.success('Add new sub community successfully')
        this.loadSubCommunities()
      }).catch((err) => {
        message.error('Error while adding new sub community')
        logger.error(err)
      })
    }

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
        this.props.history.push(`/admin/community/${communityId}/country/${geolocation}`)

        this.loadCommunityDetail(communityId)
        this.loadSubCommunities(communityId)
      } else {
        this.props.history.push('/admin/community')
      }
    }

    getListSubCommunitiesByType(subCommunities, filterRegionName) {
      let renderCommunities

      if (filterRegionName) {
        renderCommunities = subCommunities.filter((community) => {
          return community.name === filterRegionName
        })
      } else {
        renderCommunities = subCommunities
      }

      const listSubCommunitiesByType = {
        STATE: [],
        CITY: [],
        REGION: [],
        SCHOOL: [],
      }

      renderCommunities.forEach((community) => {
        listSubCommunitiesByType[community.type] = listSubCommunitiesByType[community.type] || []
        listSubCommunitiesByType[community.type].push(community)
      })

      return listSubCommunitiesByType
    }

    handleChangeRegion(region) {
      if (region) {
        const listSubCommunitiesByType = this.getListSubCommunitiesByType(this.state.subCommunities, region)
        this.setState({
          listSubCommunitiesByType
        })
        this.props.history.push(`/admin/community/${this.props.match.params.community}/country/${this.props.match.params.country}/region/${region}`)
      } else {
        this.props.history.push(`/admin/community/${this.props.match.params.community}/country/${this.props.match.params.country}`)
      }
    }

    renderListCountriesEl() {
      if (!this.state.communities) {
        return null
      }

      const communities = []
      const listCountriesEl = this.state.communities.map((country, index) => {
        if (communities.includes(country.name)) {
          return null
        }

        communities.push(country.name)
        return (
          <Select.Option title={country.name} key={index}
            value={country.name}>
            {country.name}
          </Select.Option>
        )
      })

      return listCountriesEl
    }

    renderBreadcrumbCountries() {
      const geolocationKeys = {}
      this.state.communities.forEach((community) => {
        geolocationKeys[community.geolocation] = community.geolocation
      })
      const listCountriesEl = Object.keys(geolocationKeys).map((geolocation, index) => {
        return (
          <Select.Option title={config.data.mappingCountryCodeToName[geolocation]} key={index}
            value={geolocation}>
            {config.data.mappingCountryCodeToName[geolocation]}
          </Select.Option>
        )
      })

      const menuCountriesEl = (
        <Select
          allowClear={true}
          value={this.props.match.params.country || undefined}
          showSearch={true}
          style={{width: 160}}
          placeholder="Select a country"
          optionFilterProp="children"
          onChange={this.handleChangeCountry.bind(this)}
        >
          {listCountriesEl}
        </Select>
      )

      return menuCountriesEl
    }

    renderBreadcrumbRegions() {
      const listRegionsEl = this.state.breadcrumbRegions.map((region, index) => {
        return (
          <Select.Option key={index} title={region.name} value={region.name}>{region.name}</Select.Option>
        )
      })

      const menuListRegionsEl = (
        <Select
          allowClear={true}
          value={this.props.match.params.region}
          showSearch={true}
          style={{width: 160}}
          placeholder="Select a region"
          optionFilterProp="children"
          onChange={this.handleChangeRegion.bind(this)}
        >
          {listRegionsEl}
        </Select>
      )

      return menuListRegionsEl
    }

    renderSubCommunitiesByType() {
      if (!this.state.community || !this.state.listSubCommunitiesByType) {
        return null
      }

      return (
        <Row>
          <Col span={4}
            className="user-card user-card--without-padding user-card--organizer">
            <h5 className="without-padding overflow-ellipsis" title={`${this.state.community.name} Organizers`}>
                        Country Organizers
            </h5>
            {this.state.community.leaders && this.state.community.leaders.map((leader, index) => {
              return (
                <Card
                  key={index}
                  hoverable={true}
                  onClick={this.openChangeOrganizerCountry.bind(this, leader)}
                  cover={<img alt="example" src={leader.profile.avatar}/>}
                >
                  <Card.Meta
                    title={`${leader.profile.firstName} ${leader.profile.lastName}`}
                    description={leader.country}
                  />
                </Card>
              )
            })}
            <Button className="ant-btn-ebp add-organizer" type="primary" size="small" onClick={this.showModalAddOrganizer}>Add</Button>
          </Col>
          <Col span={20} className="wrap-child-box-users">
            {Object.keys(config.data.mappingSubCommunityTypesAndName).map((communityType, index) => {
              return (
                <div key={index} className="child-box-users">
                  <Button className="ant-btn-ebp add-sub-community pull-right" type="primary" size="small" onClick={this.showModalAddSubCommunity.bind(null, COMMUNITY_TYPE[communityType])}>
                    {config.data.mappingSubCommunityTypesAndName[communityType].addNewText}
                  </Button>
                  <h4 className="without-padding">
                    {config.data.mappingSubCommunityTypesAndName[communityType].pluralName}
                    {' '}
&nbsp;
                                    (
                    {this.state.listSubCommunitiesByType[COMMUNITY_TYPE[communityType]].length}
)
                  </h4>
                  <Row>
                    {this.state.listSubCommunitiesByType[COMMUNITY_TYPE[communityType]].map((community, i) => {
                      if (!this.state.showAllSubCommunity[COMMUNITY_TYPE[communityType]] && i >= this.state.showMoreMinimum) {
                        return
                      }

                      return (
                        <Col span={4}
                          key={i}
                          className="user-card">
                          {community.leaders.map((leader, index) => {
                            return (
                              <Card
                                key={index}
                                hoverable={true}
                                onClick={this.showModalUpdateSubCommunity.bind(null, community, leader)}
                                cover={<img src={leader.profile.avatar}/>}
                              >
                                <h5>
                                  {community.name}
                                </h5>
                                <p>
                                  {`${leader.profile.firstName} ${leader.profile.lastName}`}
                                  <br/>
                                  <span className="no-info">{leader.username}</span>
                                </p>
                              </Card>
                            )
                          })}

                          {community.leaders.length === 0 && (
                            <Card
                              key={index}
                              hoverable={true}
                              onClick={this.showModalUpdateSubCommunity.bind(null, community, null)}
                              cover={<img src={DEFAULT_IMAGE.UNSET_LEADER}/>}
                            >
                              <h5>
                                {community.name}
                              </h5>
                            </Card>
                          )}
                        </Col>
                      )
                    })}
                  </Row>

                  { this.state.listSubCommunitiesByType[COMMUNITY_TYPE.STATE].length > this.state.showMoreMinimum && (
                    <div onClick={this.handleShowAllSubCommunity.bind(this, COMMUNITY_TYPE.STATE)}>
                      <Divider>
                        { this.state.showAllSubCommunity[COMMUNITY_TYPE.STATE] && (
                          <Button>
Collapse
                            <Icon type="up" />
                          </Button>
                        )}
                        { !this.state.showAllSubCommunity[COMMUNITY_TYPE.STATE] && (
                          <Button>
Expanse
                            <Icon type="down" />
                          </Button>
                        )}
                      </Divider>
                    </div>
                  )}
                </div>
              )
            })}
          </Col>
        </Row>
      )
    }

    ord_renderContent () {
      const menuCountriesEl = this.renderBreadcrumbCountries()
      const menuListRegionsEl = this.renderBreadcrumbRegions()
      const subCommunitiesByType = this.renderSubCommunitiesByType()

      return (
        <div className="p_admin_index ebp-wrap c_adminCommunity">
          <div className="ebp-header-divider" />
          <div className="d_box">
            <div className="p_admin_content">
              <Row>
                <Col span={4} className="admin-left-column wrap-box-navigator">
                  <Navigator selectedItem="communtities"/>
                </Col>
                <Col span={20} className="admin-right-column wrap-box-user">
                  <div>
                    <div className="list-leaders-of-a-country">
                      {subCommunitiesByType}

                      <ModalChangeOrganizerCountry
                        users={this.state.users}
                        wrappedComponentRef={this.saveFormChangeOrganizerRef}
                        visible={this.state.visibleModalChangeOrganizer}
                        onCancel={this.handleCancelModalChangeOrganizer}
                        onCreate={this.handleChangeOrganizerCountry}
                        handleRemoveCountry={this.handleRemoveCountry}
                      />
                      <ModalAddSubCommunity
                        users={this.state.users}
                        communityType={this.state.communityType}
                        wrappedComponentRef={this.saveFormAddSubCommunityRef}
                        visible={this.state.visibleModalAddSubCommunity}
                        onCancel={this.handleCancelModalAddSubCommunity}
                        onCreate={this.handleAddSubCommunity}
                      />
                      <ModalUpdateSubCommunity
                        users={this.state.users}
                        communityType={this.state.communityType}
                        wrappedComponentRef={this.saveFormUpdateSubCommunityRef}
                        visible={this.state.visibleModalUpdateSubCommunity}
                        onCancel={this.handleCancelModalUpdateSubCommunity}
                        onCreate={this.handleUpdateSubCommunity}
                        onDelete={this.handleDeleteSubCommunity}
                      />
                      <ModalAddOrganizer
                        users={this.state.users}
                        wrappedComponentRef={this.saveFormAddOrganizerRef}
                        visible={this.state.visibleModalAddOrganizer}
                        onCancel={this.handleCancelModalAddOrganizer}
                        onCreate={this.handleCreateOrganizer}
                      />
                    </div>
                  </div>
                </Col>
              </Row>
            </div>
          </div>
        </div>
      )
    }
}
