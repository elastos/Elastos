import React from 'react'
import I18N from '@/I18N'
import _ from 'lodash'
import styled from 'styled-components'
import './style.scss'
import MediaQuery from 'react-responsive'
import { Row, Col, Timeline, Button } from 'antd'

import {LG_WIDTH} from '../../../config/constant'
import { USER_LANGUAGE } from '@/constant'
import CRCFlow from './CRCFlow'

import { images } from './images'

export default class extends React.Component {
  render () {
    return (
      <ContainerMid className="mid-section">
        <img src={images.CRCLogoImg} alt="CRCLogoImg" />
        <MainContainer>
          <CRCTitle>
            {I18N.get('home.crc.title')}
          </CRCTitle>

          <CRCDesc>
            {I18N.get('home.crc.desc')}
            <Timeline>
              <Timeline.Item>
                <ItemTitle>{I18N.get('home.crc.list.1.date')}</ItemTitle>
                <ItemDesc>{I18N.get('home.crc.list.1.text')}</ItemDesc>
              </Timeline.Item>
              <Timeline.Item>
                <ItemTitle>{I18N.get('home.crc.list.2.date')}</ItemTitle>
                <ItemDesc>{I18N.get('home.crc.list.2.text')} - <a href="/constitution/1">{I18N.get('home.crc.list.2.link')}</a>.</ItemDesc>
              </Timeline.Item>
              <Timeline.Item>
                <ItemTitle>{I18N.get('home.crc.list.3.date')}</ItemTitle>
                <ItemDesc>{I18N.get('home.crc.list.3.text')} <a href="https://www.cyberrepublic.org/docs/#/overview/crc" target="_blank">{I18N.get('home.crc.list.3.link')}</a>.</ItemDesc>
              </Timeline.Item>
              <Timeline.Item>
                <ItemTitle>{I18N.get('home.crc.list.4.date')}</ItemTitle>
                <ItemDesc>{I18N.get('home.crc.list.4.text')} <a href="/proposals">{I18N.get('home.crc.list.4.link')}</a>.</ItemDesc>
              </Timeline.Item>
            </Timeline>
          </CRCDesc>
          <CRCFlow />

          <CRCTitle>
            {I18N.get('home.crc.regionalComm')}
          </CRCTitle>
          <CommunityImg src={images.RegionMapImg} />

          <br/>
          <br/>
          <br/>

          <CRCTitle>
            {I18N.get('home.crc.submit-suggestion.1')}
          </CRCTitle>
          <Button className="cr-btn cr-btn-primary" href="/suggestion">{I18N.get('home.crc.submit-suggestion.2')}</Button>
        </MainContainer>
      </ContainerMid>
    )
  }
}


const ContainerMid = styled.div`
  margin-top: 25px;
  padding: 50px 0;
  color: #fff;
`

const MainContainer = styled.div`
  max-width: 1200px;
  margin: 0 auto;
  text-align: center;
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
  background: url(${images.StripLightImg}) no-repeat;
  @media only screen and (max-width: ${LG_WIDTH}px) {
    width: 90%;
  }
`

const ItemTitle = styled.div`
  color: #18FFFF;
  font-size: 18px;
`

const ItemDesc = styled.div`

`