import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import MediaQuery from 'react-responsive'
import I18N from '@/I18N'
import { Menu, Affix, Badge } from 'antd'
import './style.scss'

import _ from 'lodash'

import {MAX_WIDTH_MOBILE, MIN_WIDTH_PC} from '../../../../config/constant'


export default class extends BaseComponent {

  ord_states() {
    return {
      navItem: 1
    }
  }

  handlePageChange(value) {
    this.setState({
      navItem: value
    })
  }

  handleMenuClick(item, key, keyPath) {
    const lookup = {
      profileProjects: '/profile/projects',
      profileTasks: '/profile/tasks',
      profileTeams: '/profile/teams',
      profileSubmissions: '/profile/submissions',
      profileInfo: '/profile/info',
      profileCommunities: '/profile/communities',
      profileSuggestions: '/profile/suggestion',
      profileAdminSuggestions: '/admin/suggestion',
      profileAdminPermission: '/admin/permission',
      forms: '/admin/forms',
      users: '/admin/users',
      communities: '/admin/community'
    }

    const route = lookup[item.key]
    route && this.props.history.push(route)
  }

  isProfileIncomplete() {
    const isEmptyChecks = [
      'firstName',
      'lastName',
      'country',
      'avatar'
    ]

    return !_.every(_.map(isEmptyChecks, (prop) => !_.isEmpty(this.props.user.profile[prop])))
  }

  ord_render () {
    // TODO check why we can not use redirect use this.props.history
    return (
      <div className="navigator">
        <MediaQuery minWidth={MIN_WIDTH_PC}>
          <Affix offsetTop={15}>
            {this.props.is_admin && (
            <h5 className="admin-label">
              {I18N.get('role.admin.mode')}
            </h5>
            )}
            <Menu
              className="no-padding-items"
              defaultSelectedKeys={[this.props.selectedItem]}
              onClick={this.handleMenuClick.bind(this)}
              mode="inline"
            >
              <Menu.Item key="profileInfo">
                { this.isProfileIncomplete()
                  ? <Badge status="processing" text={I18N.get('2300')}/>
                  : I18N.get('2300')
                }
              </Menu.Item>
              <Menu.Item key="profileTeams">
                {I18N.get(this.props.is_admin ? '1303' : '2302')}
              </Menu.Item>
              <Menu.Item key="profileProjects">
                {I18N.get(this.props.is_admin ? '1306' : '2305')}
              </Menu.Item>
              <Menu.Item key="profileTasks">
                {I18N.get(this.props.is_admin ? '1300' : '2301')}
              </Menu.Item>
              <Menu.Item key="profileSubmissions">
                {I18N.get(this.props.is_admin ? '1304' : '2303')}
              </Menu.Item>
              <Menu.Item key="profileSuggestions">
                {I18N.get('profile.suggestion')}
              </Menu.Item>
              {this.props.is_admin && (
              <Menu.Item key="profileAdminSuggestions">
                {I18N.get('admin.suggestion')}
              </Menu.Item>
              )}
              {this.props.is_admin && (
              <Menu.Item key="forms">
                {I18N.get('1305')}
              </Menu.Item>
              )}
              {this.props.is_admin && (
              <Menu.Item key="users">
                {I18N.get('1302')}
              </Menu.Item>
              )}
              {!this.props.is_admin && (
              <Menu.Item key="profileCommunities">
                {I18N.get('2304')}
              </Menu.Item>
              )}
              {this.props.is_admin && (
              <Menu.Item key="communities">
                {I18N.get('2306')}
              </Menu.Item>
              )}
              {this.props.is_admin && (
              <Menu.Item key="profileAdminPermission">
                {I18N.get('permission.title')}
              </Menu.Item>
              )}
            </Menu>
          </Affix>
        </MediaQuery>
        <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
          <Menu
            defaultSelectedKeys={[this.props.selectedItem]}
            onClick={this.handleMenuClick.bind(this)}
            mode="horizontal"
          >
            <Menu.Item key="profileInfo">
              {I18N.get('2300')}
            </Menu.Item>
            <Menu.Item key="profileTeams">
              {I18N.get(this.props.is_admin ? '1303' : '2302')}
            </Menu.Item>
            <Menu.Item key="profileProjects">
              {I18N.get(this.props.is_admin ? '1306' : '2305')}
            </Menu.Item>
            <Menu.Item key="profileTasks">
              {I18N.get(this.props.is_admin ? '1300' : '2301')}
            </Menu.Item>
            <Menu.Item key="profileSubmissions">
              {I18N.get(this.props.is_admin ? '1304' : '2303')}
            </Menu.Item>
            {this.props.is_admin && (
            <Menu.Item key="forms">
              {I18N.get('1305')}
            </Menu.Item>
            )}
            {this.props.is_admin && (
            <Menu.Item key="users">
              {I18N.get('1302')}
            </Menu.Item>
            )}
            {!this.props.is_admin && (
            <Menu.Item key="profileCommunities">
              {I18N.get('2304')}
            </Menu.Item>
            )}
            {this.props.is_admin && (
            <Menu.Item key="communities">
              {I18N.get('2306')}
            </Menu.Item>
            )}
          </Menu>
        </MediaQuery>
      </div>
    )
  }
}
