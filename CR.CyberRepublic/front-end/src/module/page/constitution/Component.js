import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import './style.scss'
import { Col, Row } from 'antd'
import _ from 'lodash'
import Navigator from '@/module/page/shared/ConstitutionNavigator/Container'
import MediaQuery from 'react-responsive'
import StandardPage from '../StandardPage'

export default class extends StandardPage {
  ord_props() {
    return {
    }
  }

  componentWillUnmount() {
  }

  ord_renderContent () {
    const id = _.get(this.props, 'location.state.id', 0);

    return (
      <div style={{marginTop: 80}}>
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_Profile p_admin_content">
              <MediaQuery maxWidth={720}>
                <Row>
                  <Col className="wrap-box-navigator">
                    <Navigator />
                  </Col>
                </Row>
              </MediaQuery>
              <Row>
                <MediaQuery minWidth={720}>
                  <Col span={4} className="admin-left-column wrap-box-navigator">
                    <Navigator />
                  </Col>
                </MediaQuery>
                <Col xs={{span: 24}} md={{span: 20}} className="c_ConstitutionContainer wrap-box-user">
                  <div className="content">
                    <h1 className="title">{I18N.get(`council.title.${id}`)}</h1>
                    <div dangerouslySetInnerHTML={{__html: I18N.get(`council.article.${id}`)}} />
                  </div>
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
