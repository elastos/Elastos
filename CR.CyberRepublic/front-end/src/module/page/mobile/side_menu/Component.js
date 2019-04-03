import React from 'react'
import BaseComponent from '@/model/BaseComponent'

import { Row, Col, Icon, Menu } from 'antd'

import './style'
import { Modal } from 'antd/lib/index'
import _ from 'lodash'
import I18N from '@/I18N'

import { USER_ROLE, USER_LANGUAGE } from '@/constant'

export default class extends BaseComponent {

  handleMenuClick (ev) {
    const key = ev.key
    const {isLogin} = this.props

    if (_.includes([
      'cr100',
      'crcles',
      'ambassadors',
      'login',
      'register',
      'signup',
      'profile/info',
      'profile/teams',
      'developer',
      'developer/learn',
      'community',
      'help',
      'about',
      'faq',
      'contact',
      'suggestion',
      'proposals',
      'constitution/1',
      'council'
    ], key)) {
      this.props.history.push(`/${ev.key}`)
    } else if (key === 'logout') {
      Modal.confirm({
        title: I18N.get('logout.title'),
        content: '',
        okText: I18N.get('.yes'),
        okType: 'danger',
        cancelText: I18N.get('.no'),
        onOk: () => {
          this.props.logout()
        },
        onCancel () {
        }
      })
    } else if (key === 'teams') {
      this.props.history.push('/developer/search?lookingFor=TEAM&sortBy=createdAt&sortOrder=DESC')

    } else if (key === 'blog') {

      analytics.track('BLOG_CLICKED', {
        url: location.href
      })

      let linkToBlog = 'https://blog.cyberrepublic.org'

      if (I18N.getLang() === USER_LANGUAGE.zh) {
        linkToBlog += `/${USER_LANGUAGE.zh}`
      }

      window.location.href = linkToBlog

    } else if (key === 'forum') {

      analytics.track('FORUM_CLICKED', {
        url: location.href
      })
      if (!isLogin) {
        this.props.history.push('/login?MSG_CODE=1')
      } else {
        const forumLink = `${process.env.FORUM_URL}/login`
        window.open(forumLink, '_blank')
      }
    } else if (key === 'docs') {

      analytics.track('DOCS_CLICKED', {
        url: location.href
      })

      let linkTo = 'https://cyberrepublic.org/docs/#/'

      if (I18N.getLang() === USER_LANGUAGE.zh) {
        linkTo += `${USER_LANGUAGE.zh}/`
      }

      window.location.href = linkTo

    } else if (key === 'landing') {
      this.props.history.push('/')
    }
  }

  ord_render () {

    const isLogin = this.props.user.is_login
    const hasAdminAccess = [USER_ROLE.ADMIN, USER_ROLE.COUNCIL].includes(this.props.user.role)

    // animateStyle is passed in and handled by react-motion
    return (
      <div className="c_mobileMenu" style={this.props.animateStyle}>
        <Row>
          <Col className="right-align">
            <Icon className="closeMobileMenu" type="menu-unfold" onClick={this.props.toggleMobileMenu}/>
          </Col>
        </Row>
        <Row>
          <Col className="menuContainer">
            <Menu
              onClick={this.handleMenuClick.bind(this)}
              mode="inline"
            >
              <Menu.Item key="landing">
                {I18N.get('0012')}
              </Menu.Item>
              {/*
              <Menu.Item key="teams">
                  {I18N.get('0005')}
              </Menu.Item>
              <Menu.Item key="council-secretariat">
                  {I18N.get('navigation.council')}
              </Menu.Item> */}
              {/* <Menu.Item key="council">
                  {I18N.get('council.0001')}
              </Menu.Item> */}
              <Menu.Item key="council">
                {I18N.get('navigation.council')}
              </Menu.Item>

              <Menu.Item key="constitution/1">
                {I18N.get('navigation.constitution')}
              </Menu.Item>

              <Menu.Item key="suggestion">
                {I18N.get('navigation.suggestion')}
              </Menu.Item>
              <Menu.Item key="proposals">
                {I18N.get('council.voting.proposalList')}
              </Menu.Item>
              {/* <Menu.Item key="constitution/1">
                  {I18N.get('navigation.constitution')}
              </Menu.Item> */}
              <Menu.Item key="forum">
                {I18N.get('0011')}
              </Menu.Item>
              <Menu.Item key="blog">
                {I18N.get('0110')}
              </Menu.Item>
              <Menu.Item key="docs">
                {I18N.get('navigation.docs')}
              </Menu.Item>
            </Menu>
          </Col>
        </Row>
        <Row>
          <Col className="menuContainer">
            <Menu
              onClick={this.handleMenuClick.bind(this)}
              mode="inline"
            >
              {isLogin && (
              <Menu.Item key="profile/info">
                {I18N.get('0104')}
              </Menu.Item>
              )}
              {!isLogin && (
              <Menu.Item key="login">
                {I18N.get('0201')}
              </Menu.Item>
              )}
              {!isLogin && (
              <Menu.Item key="register">
                {I18N.get('0202')}
              </Menu.Item>
              )}
              {isLogin && (
              <Menu.Item key="logout">
                {I18N.get('0204')}
              </Menu.Item>
              )}
            </Menu>
          </Col>
        </Row>
        <Row>
          <Col className="menuContainer">
            <Menu
              onClick={this.handleMenuClick.bind(this)}
              mode="inline"
            >
              {/*
                          <Menu.Item key="help">
                              {I18N.get('0007')}
                          </Menu.Item>
                          <Menu.Item key="forum">
                              {I18N.get('0011')}
                          </Menu.Item>
                          */}
            </Menu>
          </Col>
        </Row>
      </div>
    )
  }

}
