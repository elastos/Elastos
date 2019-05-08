import React from 'react'
import I18N from '@/I18N'
import _ from 'lodash'
import styled from 'styled-components'
import './style.scss'
import MediaQuery from 'react-responsive'
import { Row, Col, Timeline } from 'antd'


import {LG_WIDTH} from '../../../config/constant'
import { USER_LANGUAGE } from '@/constant'

import { images } from './images'

export default class extends React.Component {
  render () {
    return (
      <MainContainer>
        <Inner>
          <StyledRow>
            <StyledCol span={8}>
              <StyledImg src={images.MemberImg} alt="MemberImg" />
              <Desc>{I18N.get('home.crc.flow.member')}</Desc>
            </StyledCol>
            <StyledCol span={8}>
              <StyledImg src={images.MemberImg} alt="MemberImg" />
              <Desc>{I18N.get('home.crc.flow.member')}</Desc>
            </StyledCol>
            <StyledCol span={8}>
              <StyledImg src={images.MemberImg} alt="MemberImg" />
              <Desc>{I18N.get('home.crc.flow.member')}</Desc>
            </StyledCol>
          </StyledRow>
          <StyledRow>
            <StyledCol span={8}>
              <Desc><StyledImg src={images.ArrowV} alt="ArrowV" /></Desc>
              <ImgContainer>
                <StyledImg src={images.StripDarkImg} alt="StripDarkImg" style={{ position: 'absolute', left: -32, top: -10 }} />
                <StyledImg src={images.SuggestionImg} alt="SuggestionImg" />
              </ImgContainer>
              <Desc>{I18N.get('home.crc.flow.suggestion')} #1</Desc>
              <Desc><StyledImg src={images.ArrowV} alt="ArrowV" /></Desc>
            </StyledCol>
            <StyledCol span={8}>
              <Desc><StyledImg src={images.ArrowV} alt="ArrowV" /></Desc>
              <StyledImg src={images.SuggestionImg} alt="SuggestionImg" />
              <Desc>{I18N.get('home.crc.flow.suggestion')} #2</Desc>
              <Desc><StyledImg src={images.ArrowV} alt="ArrowV" /></Desc>
            </StyledCol>
            <StyledCol span={8}>
              <Desc><StyledImg src={images.ArrowV} alt="ArrowV" /></Desc>
              <StyledImg src={images.SuggestionImg} alt="SuggestionImg" />
              <Desc>{I18N.get('home.crc.flow.suggestion')} #3</Desc>
              <Desc><StyledImg src={images.ArrowV} alt="ArrowV" /></Desc>
            </StyledCol>
          </StyledRow>
          <SuggestionContainer>
            <StyledRow>
              <StyledCol span={8}>
                <StyledImg src={images.SuggestionImg} alt="SuggestionImg" />
                <Desc>{I18N.get('home.crc.flow.suggestion')} #1</Desc>
              </StyledCol>
              <StyledCol span={8}>
                <ImgContainer>
                  <StyledImg src={images.StripDarkImg} alt="StripDarkImg" style={{ position: 'absolute', right: -40, bottom: -10 }} />
                  <StyledImg src={images.SuggestionImg} alt="SuggestionImg" />
                </ImgContainer>
                <Desc>{I18N.get('home.crc.flow.suggestion')} #2</Desc>
              </StyledCol>
              <StyledCol span={8}>
                <ImgContainer>
                  <StyledImg src={images.CircleImg} alt="CircleImg" style={{ position: 'absolute', right: -22, top: -23 }} />
                  <StyledImg src={images.SuggestionImg} alt="SuggestionImg" />
                </ImgContainer>
                <Desc>{I18N.get('home.crc.flow.suggestion')} #3</Desc>
              </StyledCol>
            </StyledRow>
          </SuggestionContainer>
          <StyledRow type="flex" align="middle">
            <StyledCol span={7}>
              <StyledImg style={{ height: 94 }} src={images.ApprovedProposalImg} alt="ApprovedProposalImg" />
              <Desc>{I18N.get('home.crc.flow.approvedProposal')}</Desc>
            </StyledCol>
            <StyledCol span={2}>
              <ImgContainer style={{ height: 55}}>
                <StyledImg src={images.ArrowH} alt="ArrowH" />
              </ImgContainer>
            </StyledCol>
            <StyledCol span={6}>
              <StyledImg style={{ height: 94 }} src={images.SiteImg} alt="SiteImg" />
              <Desc>{I18N.get('home.crc.flow.proposalPage')}</Desc>
            </StyledCol>
            <StyledCol span={2}>
              <ImgContainer style={{ height: 55}}>
                <SubmitText>{I18N.get('home.crc.flow.submitProposal')}</SubmitText>
                <StyledImg src={images.ArrowH} alt="ArrowH" />
              </ImgContainer>
            </StyledCol>
            <StyledCol span={7}>
              <CouncilArrow src={images.ArrowH} alt="ArrowH" />
              <StyledImg src={images.CouncilImg} alt="CouncilImg" />
              <Desc style={{ color: '#1DE9B6' }}>{I18N.get('home.crc.flow.council')} #1</Desc>
            </StyledCol>
          </StyledRow>
          <VotingContainer>
            <VotingTitle>{I18N.get('home.crc.flow.voting')}</VotingTitle>
            <StyledRow gutter={8}>
              <StyledCol span={4}>
                <CouncilImg src={images.CouncilImg} alt="CouncilImg" />
                <Desc>{I18N.get('home.crc.flow.council')} <br /> #2</Desc>
              </StyledCol>
              <StyledCol span={4}>
                <CouncilImg src={images.CouncilImg} alt="CouncilImg" />
                <Desc>{I18N.get('home.crc.flow.council')} <br /> #3</Desc>
              </StyledCol>
              <StyledCol span={4}>
                <CouncilImg src={images.CouncilImg} alt="CouncilImg" />
                <Desc>{I18N.get('home.crc.flow.council')} <br /> #4</Desc>
              </StyledCol>
              <StyledCol span={4}>
                <CouncilImg src={images.CouncilImg} alt="CouncilImg" />
                <Desc>{I18N.get('home.crc.flow.council')} <br /> #5</Desc>
              </StyledCol>
              <StyledCol span={4}>
                <CouncilImg src={images.CouncilImg} alt="CouncilImg" />
                <Desc>{I18N.get('home.crc.flow.council')} <br /> #6</Desc>
              </StyledCol>
              <StyledCol span={4}>
                <CouncilImg src={images.CouncilImg} alt="CouncilImg" />
                <Desc>{I18N.get('home.crc.flow.council')} <br /> #7</Desc>
              </StyledCol>
            </StyledRow>
          </VotingContainer>

          <StyledImg src={images.ShapeGroupImg} alt="ShapeGroupImg" />
        </Inner>
      </MainContainer>
    )
  }
}


const MainContainer = styled.div`
  max-width: 600px;
  margin: 0 auto;
  text-align: center;
  color: #18FFFF;
  font-weight: 200;
`

const Inner = styled.div`
`

const ImgContainer = styled.div`
  position: relative;
  display: inline-block;
`

const DashedBorder = styled.div`
  border: 2px dashed #1DE9B6;
`

const SuggestionContainer = styled(DashedBorder)`
  padding: 35px 0 10px;
  margin-bottom: 50px;
`

const VotingContainer = styled(DashedBorder)`
  color: #1DE9B6;
  background-color: #031E28;
  padding: 20px;
  margin-top: 30px;
`

const StyledRow = styled(Row)`
`

const StyledCol = styled(Col)`
`

const StyledImg = styled.img`
  position: relative;
`

const CouncilImg = styled.img`
  height: 70px;
`

const CouncilArrow = styled.img`
  transform: rotate(90deg);
  position: absolute;
  left: 55px;
  top: -45px;
`

const Desc = styled.div`
  margin: 10px auto;

  @media only screen and (max-width: ${LG_WIDTH}px) {
  }
`

const SubmitText = styled.div`
  color: white;
  position: absolute;
  top: -80px;
  left: -20px;
  width: 125px;
`

const VotingTitle = styled.div`
  color: white;
  margin-bottom: 10px;
`

