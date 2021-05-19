import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Col, Layout, Menu, Row } from 'antd'
import I18N from '@/I18N'

import './style.scss'

const { Header } = Layout
const SubMenu = Menu.SubMenu
const MenuItemGroup = Menu.ItemGroup

export default class extends BaseComponent {

  buildDetailComponent () {
    const { profile } = this.props

    const logo_el = (
      <h1 className="admin-logo">EBP</h1>
    )

    const menuItems = [
      {
        text: '1000',
        link: 'admin'
      },
      {
        text: '1001',
        link: 'how-to-earn-ela'
      },
      {
        text: '1002',
        link: 'about'
      },
      {
        text: '1003',
        link: 'faq'
      },
      {
        text: '1004',
        link: 'contact'
      }
    ]

    const menuItemsEl = menuItems.map((menu, index) => {
      return (
        <li onClick={this.clickItem.bind(this, menu.link)} key={index}>
          <a>{I18N.get(menu.text)}</a>
        </li>
      )
    })

    const menu_el = (
      <ul className="admin-top-menu">{menuItemsEl}</ul>
    )

    return {
      logo_el,
      menu_el
    }
  }

  ord_render () {

    const { menu_el, logo_el } = this.buildDetailComponent()
    const isLogin = this.props.isLogin

    return (
      <Header className="c_Header" theme="light">
        <Row>
          <Col span={12}>
            {logo_el}
          </Col>
          <Col span={12}>
            {menu_el}
          </Col>
        </Row>
      </Header>
    )
  }

  clickItem (link) {
    alert(`TODO redirect to link ${link}`)
  }
}
