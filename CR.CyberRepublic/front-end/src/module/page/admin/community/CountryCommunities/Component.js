import React from 'react'
import { Link } from 'react-router-dom'
import { Button, Card, Select, Col, message, Row, Breadcrumb, Icon, List, Avatar } from 'antd'
import _ from 'lodash'
import AdminPage from '../../BaseAdmin'
import ModalAddCountry from '../../shared/ModalAddCountry/Component'
import config from '@/config'
import { USER_GENDER } from '@/constant'
import Navigator from '../../../shared/HomeNavigator/Container'
import { logger } from '@/util'
import '../style.scss'

export default class extends AdminPage {

    state = {
      visibleModalAddCountry: false,
      communities: [],
      users: []
    }

    // Modal add country
    showModalAddCountry = () => {
      this.formRefAddCountry.props.form.setFieldsValue({
        geolocation: this.props.match.params.country,
      }, () => {
        this.setState({
          visibleModalAddCountry: true,
        })
      })
    }

    handleCancelModalAddCountry = () => {
      const form = this.formRefAddCountry.props.form
      form.resetFields()

      this.setState({visibleModalAddCountry: false})
    }

    handleCreateCountry = () => {
      const form = this.formRefAddCountry.props.form

      form.validateFields((err, values) => {
        if (err) {
          return
        }

        form.resetFields()
        this.setState({visibleModalAddCountry: false})

        this.props.addCountry({
          geolocation: values.geolocation,
          leaderIds: values.leader || '',
        }).then(() => {
          message.success('Add new country successfully')
          this.loadCommunities()
        }).catch((err) => {
          message.error('Error while add country')
          logger.error(err)
        })
      })
    }

    saveFormAddCountryRef = (formRef) => {
      this.formRefAddCountry = formRef
    }

    componentDidMount() {
      super.componentDidMount()
      this.loadCommunities()
      this.props.getAllUsers().then((result) => {
        this.setState({
          users: result.list
        })
      })
    }

    loadCommunities() {
      const currentCountry = this.props.match.params.country

      if (currentCountry) {
        this.props.getSpecificCountryCommunities(currentCountry).then((communities) => {
          this.setState({
            communities
          })
        })
      } else {
        this.props.getAllCountryCommunity().then((communities) => {
          this.setState({
            communities
          })
        })
      }
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
    // TODO: this is buggy, and we should use populate?
    /*
    convertCommunitiesLeaderIdsToLeaderObjects(communities) {

        return new Promise((resolve, reject) => {
            let userIds = []
            communities.forEach((community) => {
                userIds.push(...community.leaderIds)
            })
            userIds = _.uniq(userIds)

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

                return resolve(communities)
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
        this.props.history.push(`/admin/community/${communityId}/country/${geolocation}`)
      } else {
        this.props.getAllCountryCommunity().then((communities) => {
          this.setState({
            communities
          })

          this.props.history.push('/admin/community')
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

    renderListCommunities() {
      const listCommunitiesEl = this.state.communities.map((community, index) => {
        return (
          <Col span={6} key={index} className="user-card">
            <Link to={`/admin/community/${community._id}/country/${community.geolocation}`}>
              <Card title={community.name}>
                {community.leaderIds.length ?
                  <p className="text-light-gray">Has Organizer(s)</p> :
                  <p className="highlight-text">Needs an Organizer</p>
                }
                {/* <List
                                dataSource={community.leaders}
                                renderItem={item => (
                                    <List.Item className="organizerListItem">
                                        <table>
                                            <tbody>
                                            <tr>
                                                <td className="avatar">
                                                    <Avatar size="large" icon="user" src={item.profile.avatar}/>
                                                </td>
                                                <td>
                                                    {item.profile.firstName} {item.profile.lastName}
                                                </td>
                                            </tr>
                                            </tbody>
                                        </table>
                                    </List.Item>
                                )}
                            />
                            */}
              </Card>
            </Link>
          </Col>
        )
      })

      return listCommunitiesEl
    }

    ord_renderContent () {
      const listCommunitiesEl = this.renderListCommunities()
      const menuCountriesEl = this.renderBreadcrumbCountries()

      return (
        <div className="p_admin_index ebp-wrap c_adminCommunity">
          <div className="ebp-header-divider" />
          <div className="d_box">
            <div className="p_admin_content">
              <Row>
                <Col span={4} className="wrap-box-navigator">
                  <Navigator selectedItem="community"/>
                </Col>
                <Col span={20} className="admin-right-column wrap-box-user">
                  <div>
                    <Button className="ant-btn-ebp pull-right" onClick={this.showModalAddCountry} type="primary">Add country</Button>
                    <Row className="clearfix">
                      {listCommunitiesEl}
                    </Row>

                    <ModalAddCountry
                      users={this.state.users}
                      wrappedComponentRef={this.saveFormAddCountryRef}
                      visible={this.state.visibleModalAddCountry}
                      onCancel={this.handleCancelModalAddCountry}
                      onCreate={this.handleCreateCountry}
                    />
                  </div>
                </Col>
              </Row>
            </div>
          </div>
        </div>
      )
    }
}
