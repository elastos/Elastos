import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Badge, Icon, Layout, Menu } from 'antd'
import I18N from '@/I18N'

import './style.scss'

const {Header} = Layout
const SubMenu = Menu.SubMenu
const MenuItemGroup = Menu.ItemGroup

export default class extends BaseComponent {
  ord_render () {
    return (
      <Menu
        className="admin-menu"
        onClick={this.handleClick.bind(this)}
        selectedKeys={['mail']}
        mode="horizontal"
      >
        <SubMenu title={(
          <span>
            <Icon type="caret-down"/>
            {I18N.get('1100')}
          </span>
)}>
          <Menu.Item key="setting:1">TODO Option 1</Menu.Item>
          <Menu.Item key="setting:2">TODO Option 2</Menu.Item>
        </SubMenu>
        <Menu.Item key="community">
          {I18N.get('1101')}
        </Menu.Item>
        <Menu.Item key="directory">
          {I18N.get('1102')}
        </Menu.Item>
        <Menu.Item className="menu-right" key="find">
          <Badge count={5} offset={[-5, 5]}>
            {I18N.get('1105')}
          </Badge>
        </Menu.Item>
        <Menu.Item className="menu-right" key="post">
          {I18N.get('1104')}
        </Menu.Item>
        <Menu.Item className="menu-right" key="work">
          {I18N.get('1103')}
        </Menu.Item>
      </Menu>
    )
  }

  handleClick (e) {
    this.setState({
      current: e.key
    })
  }
}
