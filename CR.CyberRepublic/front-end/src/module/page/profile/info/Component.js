import React from 'react'
import Navigator from '@/module/page/shared/HomeNavigator/Container'
import Footer from '@/module/layout/Footer/Container'
import Profile from '@/module/profile/Container'

import './style.scss'
import '../../admin/admin.scss'

import { Col, Row } from 'antd'

import MediaQuery from 'react-responsive'
import ProfilePage from '../../ProfilePage'

export default class extends ProfilePage {
  ord_renderContent() {
    return (
      <div>
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_Profile p_admin_content">
              <MediaQuery maxWidth={720}>
                <Row>
                  <Col className="wrap-box-navigator">
                    <Navigator selectedItem="profileInfo" />
                  </Col>
                </Row>
              </MediaQuery>
              <Row>
                <MediaQuery minWidth={720}>
                  <Col span={4} className="admin-left-column wrap-box-navigator">
                    <Navigator selectedItem="profileInfo" />
                  </Col>
                </MediaQuery>
                <Col xs={{span: 24}} md={{span: 20}} className="c_ProfileContainer admin-right-column wrap-box-user">
                  <Profile user={this.props.user}/>
                </Col>
              </Row>
              <Row>
                <Col>
                  <br/>
                </Col>
              </Row>
            </div>
          </div>
        </div>
        <Footer/>
      </div>
    )
  }
}
