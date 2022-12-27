import React from 'react'
import {
  Col, Row, Icon, Input,
} from 'antd'
import AdminPage from '../BaseAdmin'

import '../admin.scss'
import './style.scss'

import ListUsers from './ListUsers/Component'
import Navigator from '../../shared/HomeNavigator/Container'

export default class extends AdminPage {
    state = {
      usernameFilter: '',
    }

    async componentDidMount() {
      await super.componentDidMount()
      this.props.listUsers()
    }

    handleSearchUser(value) {
      this.setState({ usernameFilter: value })
    }

    ord_renderContent() {
      let { users } = this.props

      if (this.state.usernameFilter) {
        users = users.filter((user) => {
          const regExp = new RegExp(this.state.usernameFilter, 'i')
          return (
            regExp.test(user.username)
                    || regExp.test(user.profile.firstName)
                    || regExp.test(user.profile.lastName)
                    || regExp.test(user.email)
          )
        })
      }

      return (
        <div className="p_admin_index ebp-wrap">
          <div className="ebp-header-divider" />
          <div className="d_box">
            <div className="p_admin_content">
              <Row>
                <Col span={4} className="wrap-box-navigator">
                  <Navigator selectedItem="users" />
                </Col>
                <Col span={20} className="admin-right-column wrap-box-user">
                  <div className="pull-right">
                    <Input.Search
                      onSearch={this.handleSearchUser.bind(this)}
                      prefix={<Icon type="user" style={{ color: 'rgba(0,0,0,.25)' }} />}
                      placeholder="search user"
                    />
                  </div>
                  <div className="vert-gap-sm clearfix" />
                  <ListUsers users={users} refetch={this.props.listUsers} history={this.props.history} loading={this.props.loading} />
                </Col>
              </Row>
            </div>
          </div>
        </div>
      )
    }
}
