import React from 'react'
import BaseComponent from '@/model/BaseComponent'

import {Row, Col, Icon, Menu} from 'antd';

import './style'
import { Modal } from 'antd/lib/index'
import _ from 'lodash'
import I18N from '@/I18N'

import {USER_ROLE, USER_LANGUAGE} from '@/constant'

export default class extends BaseComponent {

    handleMenuClick(ev,) {

        const key = ev.key
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
            'community',
            'help',
            'about',
            'faq',
            'contact',
            'slack',
            'council'
        ], key)) {
            this.props.history.push('/' + ev.key)
        }
        else if (key === 'logout') {
            Modal.confirm({
                title: I18N.get('logout.title'),
                content: '',
                okText: I18N.get('.yes'),
                okType: 'danger',
                cancelText: I18N.get('.no'),
                onOk: () => {
                    this.props.logout()
                },
                onCancel() {
                }
            })
        } else if (key === 'blog') {
            let linkToBlog = 'https://blog.cyberrepublic.org'

            if (I18N.getLang() === USER_LANGUAGE.zh) {
                linkToBlog += `/${USER_LANGUAGE.zh}`
            }

            window.location.href = linkToBlog
        }
    }

    ord_render () {

        const isLogin = this.props.user.is_login
        const hasAdminAccess = [USER_ROLE.ADMIN, USER_ROLE.COUNCIL].includes(this.props.user.role)

        // animateStyle is passed in and handled by react-motion
        return <div className="c_mobileMenu" style={this.props.animateStyle}>
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
                        <Menu.Item key="cr100">
                            {I18N.get('0105')}
                        </Menu.Item>
                        <Menu.Item key="crcles">
                            {I18N.get('0106')}
                        </Menu.Item>
                        <Menu.Item key="developer">
                            {I18N.get('0102')}
                        </Menu.Item>
                        <Menu.Item key="council">
                            {I18N.get('council.0001')}
                        </Menu.Item>
                        <Menu.Item key="blog">
                            {I18N.get('0110')}
                        </Menu.Item>
                        <Menu.Item key="ambassadors">
                            {I18N.get('0107')}
                        </Menu.Item>
                        {/*<Menu.Item key="developer">
                            {I18N.get('0100')}
                        </Menu.Item>*/}
                    </Menu>
                </Col>
            </Row>
            <Row>
                <Col className="menuContainer">
                    <Menu
                        onClick={this.handleMenuClick.bind(this)}
                        mode="inline"
                    >
                        {isLogin &&
                            <Menu.Item key="profile/info">
                                {I18N.get('0104')}
                            </Menu.Item>
                        }
                        {!isLogin &&
                            <Menu.Item key="login">
                                {I18N.get('0201')}
                            </Menu.Item>
                        }
                        {!isLogin &&
                        <Menu.Item key="register">
                            {I18N.get('0202')}
                        </Menu.Item>
                        }
                        {isLogin &&
                        <Menu.Item key="logout">
                            {I18N.get('0204')}
                        </Menu.Item>
                        }
                    </Menu>
                </Col>
            </Row>
            <Row>
                <Col className="menuContainer">
                    <Menu
                        onClick={this.handleMenuClick.bind(this)}
                        mode="inline"
                    >
                        <Menu.Item key="help">
                            {I18N.get('0007')}
                        </Menu.Item>
                        <Menu.Item key="slack">
                            {I18N.get('0011')}
                        </Menu.Item>
                    </Menu>
                </Col>
            </Row>
        </div>
    }

}
