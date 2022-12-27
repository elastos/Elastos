import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import styled from 'styled-components'
import './style.scss'
import MediaQuery from 'react-responsive'
import { Row, Col } from 'antd'
import {LG_WIDTH} from '../../../config/constant'
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
              {I18N.get('home.mob.logo.title')}
              <br/>
              <br/>
              <a target="_blank" href="https://www.elastos.org">{I18N.get('home.mob.logo.subtitle')}</a>
            </CRLogoMobText>
          </CRLogoMobContainer>

          <InfoRowMob>
            <InfoImgContainerCR>
              <InfoImg src={images.CREcosystemFundImg} />
            </InfoImgContainerCR>
            <InfoDesc>
              {I18N.get('home.hero.cr.fund.1')}
              <br/>
              {I18N.get('home.hero.cr.fund.2')}
            </InfoDesc>
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
              <InfoDesc>
                {I18N.get('home.hero.ela.application.1')}
                <br/>
                {I18N.get('home.hero.ela.application.2')}
              </InfoDesc>
            </InfoColMid>
            {/* Moving this up a bit */}
            <InfoColRight span={8}>
              <InfoImgContainer>
                <InfoImg src={images.ElaSidechainImg} />
              </InfoImgContainer>
              <InfoDesc>
                {I18N.get('home.hero.ela.sidechain')}
                <br/>
(ETH, NEO, DID)
              </InfoDesc>
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
              <InfoDesc>
                {I18N.get('home.hero.cr.fund.1')}
                <br/>
                {I18N.get('home.hero.cr.fund.2')}
              </InfoDesc>
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
        <div className="mid-section">
          <div className="decoration-2">
            <img className="upper-left" src="/assets/images/training_green_slashed_box.png"/>
          </div>
          <div className="inner-box">
            <div className="decoration-3">
              <img className="upper-left" src="/assets/images/training_green_slashed_box.png"/>
            </div>
            <h3>{title}</h3>
            <p className="synthese">{I18N.get('home.crc.explanation')}</p>
            <p className="synthese">{description2}</p>
          </div>
          <div className="rectangle-1" />
          <div className="rectangle-2" />
          <div className="rectangle-3" />
        </div>
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
