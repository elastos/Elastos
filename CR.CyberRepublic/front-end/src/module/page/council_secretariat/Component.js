import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import { Col, Row, Avatar, Tabs, Divider } from 'antd'
import styled from 'styled-components'
import StandardPage from '../StandardPage'
import PersonCard from './PersonCard'
import { text, border } from '@/constants/color'
import './style.scss'

const TabPane = Tabs.TabPane

export default class extends StandardPage {
  linkToRule() {
    this.props.history.push('/constitution/1')
  }

  ord_renderContent() {
    return (
      <div className="p_cs">
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_content">
              {this.buildContent()}
            </div>
          </div>
          <div className="council-rule">
            <h3 className="title">{I18N.get('cs.rule.tile')}</h3>
            <span className="view-rule">
              {I18N.get('cs.rule.show.click')}
              {' '}
              <span className="click-here" onClick={this.linkToRule.bind(this)}>{I18N.get('cs.rule.show.here')}</span>
              {' '}
              {I18N.get('cs.rule.show.view')}
            </span>
          </div>
        </div>
        <Footer />
      </div>
    )
  }

  buildIncumbent() {
    return (
      <div className="incumbent">
        <div className="title">
          {I18N.get('cs.incumbent')}
          <span className="title_1st"><Divider style={{ backgroundColor: '#7E96BE' }} className="line" type="vertical" />1ST</span>
        </div>
        <Row className="members">
          <Col lg={8} md={8} sm={24} className="member">
            <div className="small-rect">
              <Avatar src="/assets/images/council/council-kevin.jpeg" shape="square" size={220} icon="user" />
            </div>
            <div className="big-rect">
              <div className="content">
                <h3 className="name">{I18N.get('cs.kevin.name')}</h3>
                <span className="self-intro">{I18N.get('cs.kevin.intro')}</span>
              </div>
            </div>
          </Col>
          <Col lg={8} md={8} sm={24} className="member">
            <div className="small-rect">
              <Avatar src="/assets/images/council/council-yipeng.jpeg" shape="square" size={220} icon="user" />
            </div>

            <div className="big-rect">
              <div className="content">
                <h3 className="name">{I18N.get('cs.yipeng.name')}</h3>
                <span className="self-intro">{I18N.get('cs.yipeng.intro')}</span>
              </div>
            </div>
          </Col>
          <Col lg={8} md={8} sm={24} className="member">
            <div className="small-rect">
              <Avatar src="/assets/images/council/council-feng.jpeg" shape="square" size={220} icon="user" />
            </div>

            <div className="big-rect">
              <div className="content">
                <h3 className="name">{I18N.get('cs.feng.name')}</h3>
                <span className="self-intro">{I18N.get('cs.feng.intro')}</span>
              </div>
            </div>
          </Col>
        </Row>
      </div>
    )
  }

  buildSecretariat() {
    return (
      <div className="secretariat">
        <div className="title">
          {I18N.get('cs.secretariat.general')}
          {/* <span className="title_1st"><Divider className="line" type="vertical" />1ST</span> */}
        </div>
        <Row className="members">
          <Col lg={8} md={8} sm={24} className="member">
            <div className="small-rect">
              <Avatar src="/assets/images/council/secretary-rebecca.jpeg" shape="square" size={220} icon="user" />
            </div>

            <div className="big-rect">
              <div className="content">
                <h3 className="name">{I18N.get('cs.rebecca.name')}</h3>
                <span className="self-intro">{I18N.get('cs.rebecca.intro')}</span>
              </div>
            </div>
          </Col>
        </Row>
        {/* <div className="title">{I18N.get('cs.secretariat.staff')}<span className="title_1st"><Divider className="line" type="vertical" />1ST</span></div>
          <Row className="members">
              <Col lg={8} md={8} sm={24} className="member">
                  <div className="small-rect"></div>
                  <div className="big-rect">
                      <div className="content">
                          <h3 className="name">Kevin Zhang</h3>
                          <span className="self-intro">Kevin is a dedicated industry leader, he is a advisor of BTC, Kevin is a dedicated industry leader, he is a advisor of BTC lorem ipsum dolor sit</span>
                      </div>
                  </div>
              </Col>
          </Row> */}
        {this.buildPositions()}
      </div>
    )
  }

  buildPositions() {
    return (
      <div className="positions">
        <div className="title">
          {I18N.get('cs.secretariat.positions.title')}
          {/* <span className="title_1st"><Divider className="line" type="vertical" />1ST</span> */}
        </div>
        <Row className="members">
          <PersonCard title={I18N.get('cs.secretariat.positions.position_1.title')} desc={I18N.get('cs.secretariat.positions.position_1.desc')} link="/position/secretariat" />
          <PersonCard title={I18N.get('cs.secretariat.positions.position_2.title')} desc={I18N.get('cs.secretariat.positions.position_2.desc')} link="/position/secretariat" />
          <PersonCard title={I18N.get('cs.secretariat.positions.position_3.title')} desc={I18N.get('cs.secretariat.positions.position_3.desc')} link="/position/secretariat" />
          <PersonCard title={I18N.get('cs.secretariat.positions.position_4.title')} desc={I18N.get('cs.secretariat.positions.position_4.desc')} link="/position/secretariat" />
        </Row>
      </div>
    )
  }

  buildContent() {
    return (
      <div className="cs-background">
        <div className="circle-container">
          <img className="circle" src="assets/images/council_circle.png" />
        </div>
        <div className="circle-top1">
          <img className="circle" src="assets/images/council_circle.png" />
        </div>
        <div className="circle-top2">
          <img className="circle" src="assets/images/council_circle.png" />
        </div>
        <div className="right-box-container">
          <div className="small-box" />
          <div className="box" />
          <img src="assets/images/training_green_slashed_box.png" />
        </div>
        <div className="connector-container">
          <img src="assets/images/council_connector.png" />
        </div>
        <div className="container">
          <div className="rect-container">
            <div className="rect" />
            <StyledTabs defaultActiveKey="1" tabBarStyle={{ borderBottom: 'none', color: text.middleGray }}>
              <TabPane tab={<TabTitle>{I18N.get('cs.council')}</TabTitle>} key="1">{this.buildIncumbent()}</TabPane>
              <TabPane tab={<TabTitle>{I18N.get('cs.secretariat.title')}</TabTitle>} key="2">{this.buildSecretariat()}</TabPane>
            </StyledTabs>
          </div>
        </div>
      </div>
    )
  }
}

const StyledTabs = styled(Tabs)`
  .ant-tabs-nav .ant-tabs-tab {
    border-bottom: none;
    color: ${text.middleGray};
    padding: 0;
    :first-child:after {
      content: '';
      background-color: ${border.middleGray};
      height: 40px;
      width: 1px;
      display: block;
      position: absolute;
      top: 22px;
      left: 165px;
    }
  }
  .ant-tabs-nav .ant-tabs-tab-active {
    color: ${text.green};
  }
  .ant-tabs-ink-bar {
    display: none!important;
  }
`
const TabTitle = styled.div`
  font-family: "komu-a",sans-serif;
  font-size: 64px;
`
// .title-council {
//   line-height: normal;
//   color: #1DE9B6;
// }

// .title-secretariat {
//   clear: left;
//   font-family: "komu-a",sans-serif;
//   line-height: normal;
//   font-size: 56px;
//   color: #1DE9B6;
//   margin-top: 35px;
//   margin-bottom: 50px;
// }
