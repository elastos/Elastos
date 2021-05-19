import React from 'react'
import _ from 'lodash'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import I18N from '@/I18N'
import styled from 'styled-components'
import { breakPoint } from '@/constants/breakPoint'
import { text, bg } from '@/constants/color'
import StandardPage from '../../StandardPage'
import BGImg from '../BGImg'

export default class extends StandardPage {
  ord_renderContent() {
    const { match } = this.props
    const id = _.get(match, 'params.id')
    return (
      <div className="p_cs">
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_content">
              <Container className="cs-background">
                <BGImg />
                <LinkContainer>
                  <BackLink link="/council" style={{ color: 'white' }} />
                </LinkContainer>
                <Main>
                  <Title>{I18N.get('cs.secretariat.positions.title')}</Title>
                  <PositionContainer>
                    <SubTitle>{I18N.get(`cs.secretariat.positions.position_${id}.title`)}</SubTitle>
                    <Job>
                      <JobTitle>{I18N.get('cs.secretariat.positions.jobDesc')}</JobTitle>
                      <JobDesc>{I18N.get(`cs.secretariat.positions.position_${id}.desc`)}</JobDesc>
                    </Job>
                  </PositionContainer>
                </Main>
              </Container>
              <Howto>
                <SubTitle>{I18N.get('cs.secretariat.positions.howtoApply.title')}</SubTitle>
                <HowtoDesc>{I18N.get('cs.secretariat.positions.howtoApply.desc')}</HowtoDesc>
              </Howto>
              <Footer />
            </div>
          </div>
        </div>
      </div>
    )
  }
}

const Container = styled.div`
  padding-top: 142px;
`
const LinkContainer = styled.div`
  @media only screen and (max-width: ${breakPoint.mobile}) {
    position: absolute;
    left: -10px;
    top: 100px;
  }

`
const Main = styled.div`
  margin-left: 150px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-left: 20px;
  }
`
const Title = styled.h2`
  font-family: "komu-a",sans-serif;
  font-size: 64px;
  color: ${text.green};
  padding-top: 0;
  line-height: 1;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    font-size: 38px;
  }
`
const PositionContainer = styled.div`

`
const SubTitle = styled.h3`
  font-family: "komu-a",sans-serif;
  font-size: 36px;
  color: ${text.green};
  padding-top: 0;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    font-size: 32px;
  }

`
const Job = styled.div`

`
const JobTitle = styled.h4`
  font-family: "komu-a",sans-serif;
  font-size: 30px;
  color: #FFFFFF;
`
const JobDesc = styled.div`
  color: white;
`
const Howto = styled.div`
  width: 80%;
  background-color: ${bg.middleGreen};
  padding: 30px 0 30px 150px;
  margin-bottom: 50px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    width: 100%;
    padding-left: 20px;
  }
`
const HowtoDesc = styled.div`
  color: white;
`
