import React from 'react'
import _ from 'lodash'
import { Col, Row, Avatar, Tabs } from 'antd'
import styled from 'styled-components'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import StandardPage from '../StandardPage'
import PersonCard from './PersonCard'
import BGImg from './BGImg'
import { bg, text, border } from '@/constants/color'
import { breakPoint } from '@/constants/breakPoint'
import Toast from '@/module/common/Toast'

import './style.scss'

const {TabPane} = Tabs

export default class extends StandardPage {
  constructor(props) {
    super(props)
    this.state = {
      // save the page you are on
      tab: this.props.council.tab || '1',
    }
  }

  linkToRule() {
    this.props.history.push('/whitepaper')
  }

  ord_renderContent() {
    return (
      <div className="p_cs">
        <Toast storageKey="openPositions" message={I18N.get('cs.secretariat.positions.toastMsg')} defaultDisplay={false} />

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
          {/* <span className="title_1st"><Divider style={{ backgroundColor: '#7E96BE' }} className="line" type="vertical" />1ST</span> */}
        </div>
        <Row className="members">
          <Col lg={8} md={8} sm={24} className="member">
            <div className="small-rect">
              <Avatar src="/assets/images/council/council-no1.jpeg" shape="square" size={220} icon="user" />
            </div>
            <div className="big-rect">
              <div className="content">
                <h3 className="name">{I18N.get('cs.no1.name')}</h3>
                <div className="self-intro">{I18N.get('cs.no1.intro')}</div>
                <Email>
                  {I18N.get('cs.contact')}
:
                  {' '}
                  {I18N.get('cs.no1.email')}
                </Email>
              </div>
            </div>
          </Col>
          <Col lg={8} md={8} sm={24} className="member">
            <div className="small-rect">
              <Avatar src="/assets/images/council/council-no2.jpeg" shape="square" size={220} icon="user" />
            </div>

            <div className="big-rect">
              <div className="content">
                <h3 className="name">{I18N.get('cs.no2.name')}</h3>
                <div className="self-intro">{I18N.get('cs.no2.intro')}</div>
                <Email>
                  {I18N.get('cs.contact')}
:
                  {' '}
                  {I18N.get('cs.no2.email')}
                </Email>
              </div>
            </div>
          </Col>
          <Col lg={8} md={8} sm={24} className="member">
            <div className="small-rect">
              <Avatar src="/assets/images/council/council-no3.jpeg" shape="square" size={220} icon="user" />
            </div>

            <div className="big-rect">
              <div className="content">
                <h3 className="name">{I18N.get('cs.no3.name')}</h3>
                <div className="self-intro">{I18N.get('cs.no3.intro')}</div>
                <Email>
                  {I18N.get('cs.contact')}
:
                  {' '}
                  {I18N.get('cs.no3.email')}
                </Email>
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
                <div className="self-intro">{I18N.get('cs.rebecca.intro')}</div>
              </div>
            </div>
          </Col>
        </Row>
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
          <PersonCard title={I18N.get('cs.secretariat.positions.position_1.title')} link="/position/secretariat/1" />
          <PersonCard title={I18N.get('cs.secretariat.positions.position_2.title')} link="/position/secretariat/2" />
          <PersonCard title={I18N.get('cs.secretariat.positions.position_3.title')} link="/position/secretariat/3" />
          <PersonCard title={I18N.get('cs.secretariat.positions.position_4.title')} link="/position/secretariat/4" />
        </Row>
      </div>
    )
  }

  buildContent() {
    const { tab } = this.props.council
    const tabBarStyle = { borderBottom: 'none', color: text.middleGray }
    return (
      <div className="cs-background">
        <BGImg />
        <div className="container">
          <div className="rect-container">
            <div className="rect" />
            <StyledTabs defaultActiveKey="COUNCIL" activeKey={tab} onChange={this.tabChange} tabBarStyle={tabBarStyle}>
              <TabPane tab={<TabTitle>{I18N.get('cs.council')}</TabTitle>} key="COUNCIL">{this.buildIncumbent()}</TabPane>
              <TabPane tab={<TabTitle>{I18N.get('cs.secretariat.title')}</TabTitle>} key="SECRETARIAT">{this.buildSecretariat()}</TabPane>
            </StyledTabs>
          </div>
        </div>
      </div>
    )
  }

  tabChange = (activeKey) => {
    return this.props.changeTab(activeKey)
  }
}

const StyledTabs = styled(Tabs)`
  .ant-tabs-nav .ant-tabs-tab {
    border-bottom: none;
    color: ${text.middleGray};
    padding: 0;
    :not(:last-child):after {
      content: '';
      background-color: ${border.middleGray};
      display: block;
      position: absolute;
      width: 1px;
      height: 40px;
      top: 22px;
      left: calc(100% + 16px);
      @media only screen and (max-width: ${breakPoint.mobile}) {
        height: 32px;
        top: 15px;
      }
    }
  }
  .ant-tabs-nav .ant-tabs-tab-active {
    color: ${text.green};
  }
  .ant-tabs-ink-bar {
    display: none!important;
  }
  .ant-tabs-tab-prev-icon-target, .ant-tabs-tab-next-icon-target {
    color: ${text.green};
    svg {
      width: 2em;
      height: 2em;
    }
  }
`
const TabTitle = styled.div`
  font-family: "komu-a",sans-serif;
  font-size: 64px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    font-size: 48px;
  }
`
const Email = styled.div`
  color: white;
  margin-top: 5px
`
const Voting = styled.div`
  .ant-row {
    padding-bottom: 56px;
  }
`
const Header = styled.div`
  margin: 27px 0 80px;
  width: 211px;
  height: 48px;
  font-family: 'komu-a', sans-serif;
  font-size: 48px;
  line-height: 48px;
  color: ${text.green};
`
const Card = styled.div`
  width: 201px;
  height: 325px;
  background: ${bg.darkNavy};
`
const StyledAvatar = styled.div`
  width: 176px;
  height: 176px;
  position: relative;
  top: -30px;
  background: ${bg.obsidian};
`
const Rank = styled.div`
  padding: 4px 4px 4px 6px;
  min-width: 36px;
  height: 36px;
  position: absolute;
  top: 158px;
  right: -18px;
  background: #18ffff;
  color: #000000;
  display: flex;
  justify-content: center;
`

const Number = styled.div`
  font-family: komu-a;
  font-size: 36px;
  line-height: 36px;
`
const Suffix = styled.div`
  font-family: komu-a;
  font-size: 14px;
  line-height: 14px;
`
const Info = styled.div`
  margin-top: -6px;
  padding-left: 16px;
  padding-right: 25px;
  .wrap-content {
    white-space: nowrap; 
    overflow: hidden;
    text-overflow: ellipsis; 
  }
`
const Meta = styled.div`
  height: 96px;
  margin-top: 10px;
  font-family: Synthese;
  font-size: 14px;
  line-height: 24px;
  color: #f6f9fd;
  opacity: 0.9;
  .country {
    height: 50%;
  }
  .vote {
    height: 25%;
    display: flex;
    .data {
      font-weight: bold;
      color: ${text.white};
    }
    .data-vote {
      max-width: 70%;
    }
    .data-rate {
      max-width: 25%;
    }
  }
}
`
const Name = styled.div`
  height: 30px;
  font-family: komu-a;
  font-size: 30px;
  line-height: 30px;
  color: ${text.white};
`
const StyledPagination = styled.div`
  margin-bottom: 90px;
  text-align: center;
  .ant-pagination-prev .ant-pagination-item-link,
  .ant-pagination-next .ant-pagination-item-link {
    border-color: ${bg.navy};
    background-color: ${bg.navy};
  }
  .ant-pagination-item {
    background-color: ${bg.navy};
    a {
      color: ${text.green};
    }
    &:focus,
    &:hover {
      a {
        color: ${text.white};
      }
    }
  }
  .ant-pagination-item-active a {
    color: ${text.white};
    border-bottom: 2px solid ${text.white};
  }
  .ant-pagination-jump-prev
    .ant-pagination-item-container
    .ant-pagination-item-link-icon,
  .ant-pagination-jump-next
    .ant-pagination-item-container
    .ant-pagination-item-link-icon {
    color: ${text.green};
  }
  .ant-pagination-jump-prev
    .ant-pagination-item-container
    .ant-pagination-item-ellipsis,
  .ant-pagination-jump-next
    .ant-pagination-item-container
    .ant-pagination-item-ellipsis {
    color: ${text.green};
    &:focus,
    &:hover {
      color: ${text.green};
    }
  }
`
