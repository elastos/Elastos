import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import LoginForm from '@/module/form/LoginForm/Container'
import RegisterForm from '@/module/form/RegisterForm/Container'
import I18N from '@/I18N'
import { Tabs } from 'antd'
import _ from 'lodash'

import './style.scss'

const TabPane = Tabs.TabPane

export default class extends BaseComponent {
  ord_states() {
    return {
      persist: true,
      activeKey: 'login', // login, register, post
      hideTabBar: false,
      did: ''
    }
  }

  handleSubmit() {
    sessionStorage.removeItem('registerRedirect')
    sessionStorage.removeItem('registerWelcome')
    if (_.isFunction(this.props.onHideModal)) {
      this.props.onHideModal()
    }
    this.props.history.push('/')
  }

  showPostRegLogScreen() {
    return (
      <div className="post-state">
        <h3 className="welcome-header komu-a">{I18N.get('register.welcome')}</h3>
        <div className="strike-text">
          <div className="strike-line"/>
          <p className="welcome-text synthese" onClick={this.handleSubmit.bind(this)}>
            {I18N.get('register.join_circle')}
          </p>
        </div>
        <img className="arrow-down" src="/assets/images/emp35/down_arrow.png" />
      </div>
    )
  }

  ord_render() {
    return (
      <div className="c_LoginOrRegister">
        {this.state.activeKey === 'post' ? this.showPostRegLogScreen() : (
          <Tabs activeKey={this.state.activeKey}
            onChange={(key) => {
              // eslint-disable-next-line no-undef
              analytics.track('CLICK', {
                linkText: key,
                action: `${key} tab`,
                url: location.href
              })
              this.setState({activeKey: key})
            }}
            className={!this.state.hideTabBar ? '' : 'hide-tabbar'}>
            <TabPane tab={I18N.get('0205')} key="login">
              <LoginForm
                onHideModal={this.props.onHideModal}
                onChangeActiveKey={(key, did) => {
                  this.setState({ activeKey: key, did })
                }}
              />
            </TabPane>
            <TabPane tab={I18N.get('0202')} key="register">
              <RegisterForm
                did={this.state.did}
                onHideTabBar={() => { this.setState({hideTabBar: true}) }}
                onChangeActiveKey={(key) => { this.setState({activeKey: key}) }}
              />
            </TabPane>
          </Tabs>
        )
        }
      </div>
    )
  }
}
