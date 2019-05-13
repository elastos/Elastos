import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import _ from 'lodash'
import styled from 'styled-components'
import './style.scss'
import MediaQuery from 'react-responsive'
import { Row, Col, Timeline } from 'antd'


import {LG_WIDTH} from '../../../config/constant'
import { USER_LANGUAGE } from '@/constant'
import StandardPage from '../StandardPage'
import CRC from './CRC'

import { images } from './images'

export default class extends StandardPage {

  constructor(props) {
    super(props)

    this.state = {
      selectedBox: 0,
    }
  }

  componentDidMount() {
  }

  componentWillUnmount() {
  }

  switchToBox(box) {
    this.setState({
      selectedBox: box
    })
  }

  redirectToConstitution(link) {
    this.props.history.push(`/constitution/${link}`)
  }

  ord_renderContent() {
    return (
      <div className="c_Home">
        <MediaQuery minWidth={LG_WIDTH}>
          {this.renderDesktop()}
        </MediaQuery>
        <MediaQuery maxWidth={LG_WIDTH}>
          <CRLogoMobContainer>
            <Logo src={images.CRLogo} />
            <CRLogoMobText>
              {I18N.get('home.mob.logo.title')}<br/>
              <br/>
              <a target="_blank" href="https://www.elastos.org">{I18N.get('home.mob.logo.subtitle')}</a>
            </CRLogoMobText>
          </CRLogoMobContainer>

          <InfoRowMob>
            <InfoImgContainerCR>
              <InfoImg src={images.CREcosystemFundImg} />
            </InfoImgContainerCR>
            <InfoDesc>{I18N.get('home.hero.cr.fund.1')}<br/>{I18N.get('home.hero.cr.fund.2')}</InfoDesc>
          </InfoRowMob>

          <InfoRowMob>
            <InfoImgContainerCR>
              <InfoImg src={images.CRGovernanceImg} />
            </InfoImgContainerCR>
            <InfoDesc>{I18N.get('home.hero.cr.community')}</InfoDesc>
          </InfoRowMob>

          <InfoRowMob>
            <InfoImgContainerCR>
              <InfoImg src={images.CRDPoSImg} />
            </InfoImgContainerCR>
            <InfoDesc>{I18N.get('home.hero.cr.supernodes')}</InfoDesc>
          </InfoRowMob>
        </MediaQuery>
        <CRC />
        {this.renderWhatIsCR()}
      </div>
    )
  }

  renderDesktop() {
    return (
      <MainContainer>
        <ElaContainer>
          <LogoContainer>
            {/* Be nice if this opened a modal explaining Elastos first */}
            <a target="_blank" href="https://www.elastos.org">
              <Logo src={images.ElaLogo}/>
            </a>
          </LogoContainer>
          <ElaRow>
            <InfoCol span={8}>
              <InfoImgContainer>
                <InfoImg src={images.ElaBlockchainImg}/>
              </InfoImgContainer>
              <InfoDesc>{I18N.get('home.hero.ela.blockchain')}</InfoDesc>
            </InfoCol>
            <InfoColMid span={8}>
              <InfoImgContainer>
                <InfoImg src={images.ElaApplicationImg}/>
              </InfoImgContainer>
              <InfoDesc>{I18N.get('home.hero.ela.application.1')}<br/>{I18N.get('home.hero.ela.application.2')}</InfoDesc>
            </InfoColMid>
            {/* Moving this up a bit */}
            <InfoColRight span={8}>
              <InfoImgContainer>
                <InfoImg src={images.ElaSidechainImg} />
              </InfoImgContainer>
              <InfoDesc>{I18N.get('home.hero.ela.sidechain')}<br/>(ETH, NEO, DID)</InfoDesc>
            </InfoColRight>
          </ElaRow>
        </ElaContainer>
        <CRContainer>
          <LogoContainer>
            <Logo src={images.CRLogo}/>
          </LogoContainer>
          <CRRow>
            <InfoCol span={9}>
              <InfoImgContainerCR>
                <InfoImg src={images.CREcosystemFundImg} />
              </InfoImgContainerCR>
              <InfoDesc>{I18N.get('home.hero.cr.fund.1')}<br/>{I18N.get('home.hero.cr.fund.2')}</InfoDesc>
            </InfoCol>
            <InfoCol span={8}>
              <InfoImgContainerCR>
                <InfoImg src={images.CRGovernanceImg} />
              </InfoImgContainerCR>
              <InfoDesc>{I18N.get('home.hero.cr.community')}</InfoDesc>
            </InfoCol>
            <InfoCol span={7}>
              <InfoImgContainerCR>
                <InfoImg src={images.CRDPoSImg} />
              </InfoImgContainerCR>
              <InfoDesc>{I18N.get('home.hero.cr.supernodes')}</InfoDesc>
            </InfoCol>
          </CRRow>
        </CRContainer>
        <ClearFix/>
      </MainContainer>
    )
  }

  renderWhatIsCR() {
    const selectedBox = this.state.selectedBox
    const title = I18N.get(`home.box_${(selectedBox + 1).toString()}.title`)
    const description1 = I18N.get(`home.explanation_${(selectedBox + 1).toString()}.part_1`)
    const description2 = I18N.get(`home.explanation_${(selectedBox + 1).toString()}.part_2`)

    return (
      <div>
        <Row className="top-section" type="flex" justify="center" gutter={32}>
          <Col className={`box-wrap ${selectedBox === 0 ? 'selected-box' : ''}`} xs={24} sm={24} md={24} lg={8} onClick={this.switchToBox.bind(this, 0)}>
            <div className="box box-hover">
              <TriColTitle>{I18N.get('home.box_1.title')}<br/><br/></TriColTitle>
              <TriColDesc className={`synthese${selectedBox === 0 ? ' selected-text' : 0}`}>{I18N.get('home.box_1.description')}</TriColDesc>
            </div>
            <div className="container">
              <div className={`cuttoff-box${selectedBox === 0 ? '' : ' cutoff-box-hidden'}`} />
            </div>
            <img className={`arrow${selectedBox === 0 ? '' : ' arrow-hidden'}`} src="/assets/images/emp35/down_arrow.png"/>
          </Col>
          <Col className={`box-wrap ${selectedBox === 1 ? 'selected-box' : ''}`} xs={24} sm={24} md={24} lg={8} onClick={this.switchToBox.bind(this, 1)}>
            <div className="box box-hover">
              <TriColTitle>{I18N.get('home.box_2.title')}</TriColTitle>
              <TriColDesc className={`synthese${selectedBox === 1 ? ' selected-text' : ''}`}>{I18N.get('home.box_2.description')}</TriColDesc>
            </div>
            <div className="container">
              <div className={`cuttoff-box${selectedBox === 1 ? '' : ' cutoff-box-hidden'}`} />
            </div>
            <img className={`arrow${selectedBox === 1 ? '' : ' arrow-hidden'}`} src="/assets/images/emp35/down_arrow.png"/>
          </Col>
          <Col className={`box-wrap ${selectedBox === 2 ? 'selected-box' : ''}`} xs={24} sm={24} md={24} lg={8} onClick={this.switchToBox.bind(this, 2)}>
            <div className="box box-hover">
              <TriColTitle>{I18N.get('home.box_3.title')}</TriColTitle>
              <TriColDesc className={`synthese${selectedBox === 2 ? ' selected-text' : ''}`}>{I18N.get('home.box_3.description')}</TriColDesc>
            </div>
            <div className="container">
              <div className={`cuttoff-box${selectedBox === 2 ? '' : ' cutoff-box-hidden'}`} />
            </div>
            <img className={`arrow${selectedBox === 2 ? '' : ' arrow-hidden'}`} src="/assets/images/emp35/down_arrow.png"/>
          </Col>
        </Row>
        {selectedBox !== 2 ? (
          <div className="mid-section">
            <div className="decoration-2">
              <img className="upper-left" src="/assets/images/training_green_slashed_box.png"/>
            </div>
            <div className="inner-box">
              <div className="decoration-3">
                <img className="upper-left" src="/assets/images/training_green_slashed_box.png"/>
              </div>
              <h3>{title}</h3>
              <p className="synthese">{description1}</p>
              <p className="synthese">{description2}</p>
            </div>
            <div className="rectangle-1" />
            <div className="rectangle-2" />
            <div className="rectangle-3" />
          </div>
        ) : (
          <div className="mid-section constitution">
            <div className="row">
              <div className="col" onClick={this.redirectToConstitution.bind(this, 1)}>
                <div>
                  <h3>{I18N.get('home.explanation_3.box_1.title')}</h3>
                  <p>{I18N.get('home.explanation_3.box_1.text')}</p>
                  <span className="date">{I18N.get('home.explanation_3.date')}</span>
                  <div className="komu-a order-num">01</div>
                </div>
              </div>
              <div className="col" onClick={this.redirectToConstitution.bind(this, 2)}>
                <div>
                  <h3>{I18N.get('home.explanation_3.box_2.title')}</h3>
                  <p>{I18N.get('home.explanation_3.box_2.text')}</p>
                  <span className="date">{I18N.get('home.explanation_3.date')}</span>
                  <div className="komu-a order-num">02</div>
                </div>
              </div>
              <div className="col" onClick={this.redirectToConstitution.bind(this, 3)}>
                <div>
                  <h3>{I18N.get('home.explanation_3.box_3.title')}</h3>
                  <p>{I18N.get('home.explanation_3.box_3.text')}</p>
                  <span className="date">{I18N.get('home.explanation_3.date')}</span>
                  <div className="komu-a order-num">03</div>
                </div>
              </div>
              <div className="col" onClick={this.redirectToConstitution.bind(this, 4)}>
                <div>
                  <h3>{I18N.get('home.explanation_3.box_4.title')}</h3>
                  <p>{I18N.get('home.explanation_3.box_4.text')}</p>
                  <span className="date">{I18N.get('home.explanation_3.date')}</span>
                  <div className="komu-a order-num">04</div>
                </div>
              </div>
            </div>
            <div className="rectangle-1" />
            <div className="rectangle-2" />
            <div className="rectangle-3" />
          </div>
        )}
        <div className="stay-updated">
          <div className="form-wrap footer-email">
            <p>{I18N.get('landing.footer.note')}</p>
            <form id="footer-form" className="signup-form" name="mailing-list" action="https://cyberrepublic.us19.list-manage.com/subscribe/post-json?u=acb5b0ce41bfe293d881da424&id=272f303492"
                  method="get">
              <div className="email-wrap">
                <input type="email" name="EMAIL" data-type="req" placeholder={I18N.get('landing.footer.email')}/>
                <button type="submit" className="arrow-submit">
                  <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 17 34">
                    <polygon points="0 0 0 33.487 16.744 16.744 0 0" style={{fill: '#1de9b6'}}/>
                    <polygon points="0 24.579 7.835 16.744 0 8.91 0 24.579" className="small-tri"/>
                  </svg>
                </button>
              </div>
            </form>
          </div>
        </div>
        <Footer/>
      </div>
    )
  }
}

const TriColTitle = styled.h3`
  line-height: 1.1;
`

const TriColDesc = styled.p`
  font-weight: 200;
  font-size: 18px;
`

const MainContainer = styled.div`
  /* max-width: 1200px; */
  margin: 0 auto;
  text-align: center;
`

const LogoContainer = styled.div`
  height: 300px;
  display: flex;
  align-items: center;
  justify-content: center;
  position: relative;
  :after {
    content: '';
    height: 70%;
    width: 1px;
    position: absolute;
    right: 0;
    z-index: 1000;
    bottom: 0;
    background-color: #e5e5e5;
  }
`

const ElaContainer = styled.div`
  width: 50%;
  float: left;
`

const Logo = styled.img`
`

const ElaRow = styled(Row)`
  background-color: rgba(124, 127, 134, 0.1);
  padding: 40px;
  display: flex;
  align-items: center;
`

const CRRow = styled(ElaRow)`
  background-color: #F6F9FD;;
`

const CRContainer = styled.div`
  overflow: hidden;
`

const InfoCol = styled(Col)`
`

const InfoColRight = styled(InfoCol)`
  background-position: center left;
`

const InfoColMid = styled(InfoCol)`
  background: none;
`

const InfoImgContainer = styled.div`
  height: 90px;
  display: flex;
  align-items: center;
`

const InfoImgContainerCR = styled.div`
  height: 90px;
  display: flex;
  align-items: center;
`

const InfoImg = styled.img`
  max-width: 90%;
  margin: 0 auto;
  display: block;
`

const InfoDesc = styled.div`
  line-height: 1.8;
`

const ClearFix = styled.div`
  clear: both;
`

const CRLogoMobText = styled.div`
  font-weight: 200;
  text-align: center;
  margin-top: 20px;
`

const InfoRowMob = styled.div`
  margin-top: 24px;
  padding: 36px 0 12px 0;
  border-top: 2px solid #e0e0e0;
  text-align: center;
`

const CRLogoMobContainer = styled.div`
  margin-top: 24px;
  padding: 36px 0 12px 0;
  text-align: center;
`
