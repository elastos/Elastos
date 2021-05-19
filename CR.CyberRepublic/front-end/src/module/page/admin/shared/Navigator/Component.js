import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Menu, SubMenu } from 'antd'
import I18N from '@/I18N'
import { Link } from 'react-router-dom'

import './style.scss'

export default class extends BaseComponent {
  ord_render () {
    // TODO check why we can not use redirect use this.props.history
    return (
      <Menu
        defaultSelectedKeys={[this.props.selectedItem]}
        mode="inline"
      >
        <Menu.Item key="tasks">
          <Link to="/admin/tasks">{I18N.get('1300')}</Link>
        </Menu.Item>
        <Menu.Item key="community">
          <Link to="/admin/community">{I18N.get('2304')}</Link>
        </Menu.Item>
        <Menu.Item key="users">
          <Link to="/admin/users">{I18N.get('1302')}</Link>
        </Menu.Item>
        <Menu.Item key="teams">
          <Link to="/admin/teams">{I18N.get('1303')}</Link>
        </Menu.Item>
        <Menu.Item key="submissions">
          <Link to="/admin/submissions">{I18N.get('1304')}</Link>
        </Menu.Item>
        <Menu.Item key="forms">
          <Link to="/admin/forms">{I18N.get('1305')}</Link>
        </Menu.Item>
        <Menu.Item key="cr100">
          <Link to="/admin/cr100">{I18N.get('0105')}</Link>
        </Menu.Item>
      </Menu>
    )
  }
}
