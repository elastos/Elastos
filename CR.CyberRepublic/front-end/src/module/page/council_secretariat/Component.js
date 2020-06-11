import React from 'react'
import { Col, Row, Avatar, Tabs, Button, Popover } from 'antd'
import styled from 'styled-components'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import StandardPage from '../StandardPage'
import PersonCard from './PersonCard'
import BGImg from './BGImg'
import { text, border } from '@/constants/color'
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

  async componentWillMount() {
    const data = await this.props.getCouncilsAndSecretariat()
    this.setState({councils: data.councils, secretariat: data.secretariat})
  }

  ord_renderContent() {
    const toastMsg = (
      <div>
        {I18N.get('cs.secretariat.positions.toastMsg')}
        <Button
          className="btn-view-open"
          onClick={async () => {
            this.tabChange('SECRETARIAT')
            window.scrollTo(0, document.getElementById('open-positions').offsetTop)
          }}
        >
          {I18N.get('cs.secretariat.positions.open')}
        </Button>
      </div>
    )
    return (
      <div className="p_cs">
        <Toast
          storageKey="openPositions"
          message={toastMsg}
          defaultDisplay={false}
        />

        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_content">{this.buildContent()}</div>
          </div>
          <div className="council-rule">
            <h3 className="title">{I18N.get('cs.rule.tile')}</h3>
            <span className="view-rule">
              {I18N.get('cs.rule.show.click')}
              {' '}
              <span className="click-here" onClick={this.linkToRule.bind(this)}>
                {I18N.get('cs.rule.show.here')}
              </span>
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
    const {councils} = this.state
    const lang = localStorage.getItem('lang') || 'en'
    
    return (
      <div className="incumbent">
        <div className="title">{I18N.get('cs.incumbent')}</div>
        <Row className="members">
          { councils !== undefined ? councils.councilMembers.map(item => (
            <Col lg={8} md={8} sm={24} className="member" key={item.index}>
              <div className="small-rect">
                <Avatar
                  src={item.avatar}
                  shape="square"
                  size={220}
                  icon="user"
                />
              </div>

              <div className="big-rect">
                <div className="content">
                  <h3 className="name">{item.didName}</h3>
                  
                  <div className="self-intro" >
                    <Popover 
                      content={
                        lang === 'en' ? item.introduction ?
                        item.introduction.split('\n').length > 1 ? 
                        item.introduction.split('\n')[0]
                        : 
                        item.introduction.split('\n')[0]
                        : null
                        :
                        item.introduction ?
                        item.introduction.split('\n').length > 1 ? 
                        item.introduction.split('\n')[1]
                        : 
                        item.introduction.split('\n')[0]
                        : null}
                        title={I18N.get('cs.intro')}
                        overlayStyle={{width: 400+'px', padding: 10+'px',wordBreak: 'keep-all'}}
                    >
                      { lang === 'en' ? item.introduction ?
                        item.introduction.split('\n').length > 1 ? 
                        item.introduction.split('\n')[0]
                        : 
                        item.introduction.split('\n')[0]
                        : null
                        :
                        item.introduction ?
                        item.introduction.split('\n').length > 1 ? 
                        item.introduction.split('\n')[1]
                        : 
                        item.introduction.split('\n')[0]
                        : null
                      }
                    </Popover>
                  </div>
                  <Email>
                  <Popover 
                      content={item.email}
                      placement="topLeft"
                  >
                    {I18N.get('cs.contact')}
                    :
                    {' '}
                    {item.email}
                    </Popover>
                  </Email>
                </div>
              </div>
            </Col>
          )) : null}
        </Row>
      </div>
    )
  }

  buildSecretariat() {
    const secretariat = this.state.secretariat
    return (
      <div className="secretariat">
        <div className="title">{I18N.get('cs.secretariat.general')}</div>
        <Row className="members">
          <Col lg={8} md={8} sm={24} className="member">
            <div className="small-rect">
              <Avatar
                src="/assets/images/council/secretary-rebecca.jpeg"
                shape="square"
                size={220}
                icon="user"
              />
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
      <div id="open-positions" className="positions">
        <div className="title">
          {I18N.get('cs.secretariat.positions.title')}
        </div>
        <Row className="members">
          {[1, 2, 3, 4].map(item => (
            <PersonCard
              key={item}
              title={I18N.get(
                `cs.secretariat.positions.position_${item}.title`
              )}
              link={`/position/secretariat/${item}`}
            />
          ))}
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
            <StyledTabs
              defaultActiveKey="COUNCIL"
              activeKey={tab}
              onChange={this.tabChange}
              tabBarStyle={tabBarStyle}
            >
              <TabPane
                tab={<TabTitle>{I18N.get('cs.council')}</TabTitle>}
                key="COUNCIL"
              >
                {this.buildIncumbent()}
              </TabPane>
              <TabPane
                tab={<TabTitle>{I18N.get('cs.secretariat.title')}</TabTitle>}
                key="SECRETARIAT"
                forceRender={true}
              >
                {this.buildSecretariat()}
              </TabPane>
            </StyledTabs>
          </div>
        </div>
      </div>
    )
  }

  tabChange = activeKey => {
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
    display: none !important;
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
  font-family: 'komu-a', sans-serif;
  font-size: 64px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    font-size: 48px;
  }
`
const Email = styled.div`
  position: absolute;
  bottom: 17px;
  width: 220px;
  white-space: nowrap; 
  overflow: hidden;
  text-overflow: ellipsis; 
`
