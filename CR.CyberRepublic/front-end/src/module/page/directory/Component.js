import React from 'react'
import { Col, Row } from 'antd'
import StandardPage from '../StandardPage'
import I18N from '@/I18N'
import Footer from '@/module/layout/Footer/Container'

import './style.scss'

export default class extends StandardPage {

  ord_renderContent () {

    return (
      <div className="p_Directory">
        <div className="ebp-header-divider" />
        <div className="ebp-page">
          <div className="ebp-page-title" />

          <Row>
            <Col span={18}>
              <div className="d_leadersList">
                <h3>
                  {I18N.get('0003')}
                </h3>
              </div>
            </Col>
            <Col span={6}>
              <br/>
              <div className="ebp-gray-box center">
                <h4 />
              </div>
            </Col>
          </Row>
        </div>
        <Footer />
      </div>
    )
  }
}
