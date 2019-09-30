import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import { Avatar } from 'antd'
import styled from 'styled-components'
import StandardPage from '../StandardPage'
import { bg, text } from '@/constants/color'
import Connector from './svg/Connector'
import Square from './svg/Square'
import Circle from './svg/Circle'

export default class extends StandardPage {
  ord_renderContent() {
    return (
      <Wrapper>
        <StyledConnector />
        <StyledSquare />
        <Circles>
          <Circle />
          <Circle />
        </Circles>
        <Container>
          <Header>{I18N.get('cs.candidates')}</Header>
          <Content>
            {this.renderCandidate()}
            {this.renderCandidate()}
          </Content>
        </Container>
        <StyledCircle />
        <Footer />
      </Wrapper>
    )
  }

  renderCandidate = () => (
    <Card>
      <StyledAvatar
        src="/assets/images/council/council-yipeng.jpeg"
        shape="square"
        size={176}
        icon="user"
      />
      <Rank>
        <Number>1</Number>
        <Suffix>st</Suffix>
      </Rank>
      <Info>
        <Name>NICKNAME</Name>
        <Meta>
          <Location>Country, Region</Location>
          <div>302 <span>votes</span></div>
          <div>50% <span>of total votes</span></div>
        </Meta>
      </Info>
    </Card>
  )
}

const Wrapper = styled.div`
  background: ${bg.navy};
  position: relative;
`
const StyledConnector = styled(Connector)`
  margin-top: 40px;
`
const StyledSquare = styled(Square)`
  position: absolute;
  right: 60px;
  top: 50px;
`
const StyledCircle = styled(Circle)`
  position: absolute;
  left: 40px;
  top: calc(100% / 2 - 48px)
`
const Circles = styled.div`
  position: absolute;
  right: 345px;
  top: 117px;
`
const Container = styled.div`
  max-width: 1400px;
  margin: 0 auto;
  padding: 0 16px;
`
const Header = styled.div`
  margin: 27px 0 80px;
  width: 211px;
  height: 64px;
  font-family: 'komu-a', sans-serif;
  font-size: 64px;
  line-height: 64px;
  color: ${text.green};
`
const Content = styled.div`
  display: flex;
  margin: 0 -16px;
  flex-wrap: wrap;
`
const Card = styled.div`
  width: 208px;
  min-height: 325px;
  background: ${bg.darkNavy};
  position: relative;
  margin: 32px 16px;
`
const StyledAvatar = styled(Avatar)`
  background: ${bg.obsidian};
  position: absolute;
  top: -30px;
`
const Rank = styled.div`
  font-family: 'komu-a', sans-serif;
  position: absolute;
  background: #18ffff;
  top: 122px;
  right: 14px;
  color: #000000;
  padding: 5px 14px 0 9px;
`

const Number = styled.div`
  font-family: 'komu-a', sans-serif;
  font-size: 36px;
  display: inline-block;
  line-height: 1;
`
const Suffix = styled.div`
  position: absolute;
  font-size: 14px;
  font-family: 'komu-a', sans-serif;
  text-transform: uppercase;
  display: inline-block;
  top: 4px;
`
const Info = styled.div`
  padding: 8px 16px 19px;
`
const Meta = styled.div`
  font-family: Synthese;
  font-size: 14px;
  line-height: 24px;
  color: #f6f9fd;
  span {
    font-family: Synthese;
    opacity: .7;
  }
`
const Name = styled.div`
  font-family: 'komu-a', sans-serif;
  font-size: 30px;
  line-height: 30px;
  color: ${text.white};
`
const Location = styled.div`
  margin: 16px 0 32px;
  opacity: .7;
  font-family: Synthese;
`
