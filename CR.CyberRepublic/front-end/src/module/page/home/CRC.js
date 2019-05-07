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
        <ContainerMid className="mid-section">
          <MainContainer>
            <CRCTitle>
              {I18N.get('home.crc.title')}
            </CRCTitle>

            <CRCDesc>
              {I18N.get('home.crc.desc')}
              <Timeline>
                <Timeline.Item>
                  <b>{I18N.get('home.crc.list.1.date')}</b> - {I18N.get('home.crc.list.1.text')}
                </Timeline.Item>
                <Timeline.Item>
                  <b>{I18N.get('home.crc.list.2.date')}</b> - {I18N.get('home.crc.list.2.text')} - <a href="/constitution/1">{I18N.get('home.crc.list.2.link')}</a>.
                </Timeline.Item>
                <Timeline.Item>
                  <b>{I18N.get('home.crc.list.3.date')}</b> - {I18N.get('home.crc.list.3.text')} <a href="https://www.cyberrepublic.org/docs/#/overview/crc" target="_blank">{I18N.get('home.crc.list.3.link')}</a>.
                </Timeline.Item>
                <Timeline.Item>
                  <b>{I18N.get('home.crc.list.4.date')}</b> - {I18N.get('home.crc.list.4.text')} <a href="/proposals">{I18N.get('home.crc.list.4.link')}</a>.
                </Timeline.Item>
              </Timeline>
            </CRCDesc>
            <CommunityImg src={I18N.getLang() === USER_LANGUAGE.zh ? images.CommunityPowerZhImg : images.CommunityPowerImg}/>

            <br/>
            <br/>
            <br/>

            <CRCTitle>
              {I18N.get('home.crc.submit-suggestion.1')} <a href="/suggestion">{I18N.get('home.crc.submit-suggestion.2')}</a>
            </CRCTitle>
          </MainContainer>
        </ContainerMid>
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
  max-width: 1200px;
  margin: 0 auto;
  text-align: center;
`

const LogoContainer = styled.div`
  height: 300px;
`

const ElaContainer = styled.div`
  width: 50%;
  float: left;

  background-image: url(${images.SepCenterImg});
  background-repeat: no-repeat;
  background-size: contain;
  background-position: bottom right;
`

const ElaLogo = styled.img`
  display: block;
  margin: 0 auto;
  padding-top: 100px;
  max-width: 190px;
`

const CRLogo = styled.img`
  width: 100%;
  display: block;
`

const CRContainer = styled.div`
  overflow: hidden;
`

const InfoCol = styled(Col)`
  margin-top: 25px;
  padding: 30px 0 30px 0;
  background-image: url(${images.SepGreyImg});
  background-repeat: no-repeat;
  background-size: contain;
  background-position: center right;
`

const InfoColRight = styled(InfoCol)`
  background-position: center left;
`

const InfoColMid = styled(InfoCol)`
  background: none;
`

const InfoImgContainer = styled.div`
  height: 60px;
`

const InfoImgContainerCR = styled.div`
  height: 90px;
`

const InfoColCR = styled(InfoCol)`
  background-image: url(${images.SepBlueImg});
  padding: 0 0 30px 0;
`

const InfoColCRMid = styled(InfoColCR)`
  background: none;
  padding: 0 0 55px 0;
`

const InfoColCRRight = styled(InfoColCR)`
  background-position: center left;
  padding: 0 0 55px 0;
`

const InfoImg = styled.img`
  max-width: 90%;
  margin: 0 auto;
  display: block;
`

const InfoDesc = styled.div`
  line-height: 1.8;
`

const ContainerMid = styled.div`
  margin-top: 25px;
  padding: 50px 0;
  color: #fff;
`

const ClearFix = styled.div`
  clear: both;
`

const CommunityImg = styled.img`
  width: 70%;
  margin: 0 auto;
  display: block;

  @media only screen and (max-width: ${LG_WIDTH}px) {
    width: 95%;
  }
`

const CRCTitle = styled.div`
  color: #ccc;
  font-family: komu-a, Synthese, sans-serif;
  font-size: 48px;
  text-align: center;

  > a {
    font-family: komu-a, Synthese, sans-serif;
    font-size: 48px;
  }
`

const CRCDesc = styled.div`
  text-align: left;
  width: 70%;
  margin: 25px auto;

  @media only screen and (max-width: ${LG_WIDTH}px) {
    width: 90%;
  }
`

const CRCLIst = styled.ul`
  font-weight: 200;
  margin: 24px 0 48px 24px;

  > li {
    margin: 16px 0;

    > b {
      font-weight: 400;
    }
  }
`

const CRLogoMobText = styled.div`
  position: absolute;
  left: 50%;
  bottom: 0px;
  font-weight: 200;
  line-height: 1;

  text-align: center;
`

const InfoRowMob = styled.div`
  margin-top: 24px;
  padding: 36px 0 12px 0;
  border-top: 2px solid #e0e0e0;
  text-align: center;
`

const CRLogoMobContainer = styled.div`
  position: relative;
  padding-bottom: 24px;
`
