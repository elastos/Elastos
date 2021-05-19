import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Layout, Menu, Input, Row, Col,
} from 'antd'
import I18N from '@/I18N'
import './style.scss'

const { Header } = Layout

export default class extends BaseComponent {
    mkHeaderMenu = () => (
      <Menu className="c_Header_Menu pull-right" mode="horizontal">
        <Menu.Item className="c_MenuItem link" key="cr100">
          {I18N.get('0105')}
        </Menu.Item>

        <Menu.Item className="c_MenuItem link" key="crcles">
          {I18N.get('0106')}
        </Menu.Item>

        <Menu.Item className="c_MenuItem link" key="ambassadors">
          {I18N.get('0107')}
        </Menu.Item>

        <Menu.Item className="c_MenuItem link" key="developer">
          {I18N.get('0102')}
        </Menu.Item>

        { this.props.isLogin
          ? (
            <Menu.Item className="c_MenuItem link" key="profile">
              {I18N.get('0104')}
            </Menu.Item>
          )
          : (
            <Menu.Item className="c_MenuItem link" key="login">
              {I18N.get('0201')}
            </Menu.Item>
          )
        }
      </Menu>
    )

    mkSideNav = () => (
      <Menu
        mode="vertical"
      >
        <Menu.Item key="signUp">
          {'Sign Up'}
        </Menu.Item>
        <Menu.Item key="vision">
          {'Vision'}
        </Menu.Item>
        <Menu.Item key="whoWeAre">
          {'Who we are'}
        </Menu.Item>
      </Menu>
    )

    ord_render() {
      return (
        <div className="p_Flyout">
          <Layout>
            <Header>
              <Row>
                <Col span={12}>
                  <img width={156} height={74} src="/assets/images/ela_hamburger.png" alt="Cyber Republic" />
                </Col>
                <Col span={12}>
                  {this.mkHeaderMenu()}
                </Col>
              </Row>
            </Header>
            <Layout>
              <Row>
                <Col className="p_Flyout_Sidenav" span={10}>
                  {this.mkSideNav()}
                </Col>
                <Col className="p_Flyout_Content" span={14}>
                  <div className="intro-content">
                                    Lorem ipsum dolor sit amet, consectetur adipisicing elit. Eligendi non quis exercitationem culpa nesciunt nihil aut nostrum explicabo reprehenderit optio amet ab temporibus asperiores quasi cupiditate. Voluptatum ducimus voluptates voluptas?
                  </div>
                  <div className="intro-heading">
                                    Stay up to date with Cyber Republic
                  </div>
                  <Input placeholder="Enter Email" />
                  <div className="intro-heading">
                                    Resources
                  </div>
                  <Row>
                    <Col span={6}>Resources</Col>
                    <Col span={18}>Cyber Republic cyberrepublic@elastos.org</Col>
                  </Row>
                  <Row>
                    <Col span={6}>Resources</Col>
                    <Col span={18}>Cyber Republic cyberrepublic@elastos.org</Col>
                  </Row>
                  <Row>
                    <Col span={6}>Resources</Col>
                    <Col span={18}>Cyber Republic cyberrepublic@elastos.org</Col>
                  </Row>
                  <Row>
                    <Col span={6}>Resources</Col>
                    <Col span={18}>Cyber Republic cyberrepublic@elastos.org</Col>
                  </Row>
                  <Row>
                    <Col span={6}>Resources</Col>
                    <Col span={18}>Cyber Republic cyberrepublic@elastos.org</Col>
                  </Row>
                </Col>
              </Row>
            </Layout>
          </Layout>
        </div>
      )
    }
}
