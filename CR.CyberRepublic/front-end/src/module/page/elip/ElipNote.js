import React from 'react'
import { Row, Col } from 'antd'
import styled from 'styled-components'
import I18N from '@/I18N'
import { breakPoint } from '@/constants/breakPoint'
import NoteIcon from './NoteIcon'

const ElipNote = () => (
  <StyledRow>
    <StyledCol span={3} />
    <Col span={17}>
      <Wrapper>
        <Image>
          <NoteIcon />
        </Image>
        <Text>
          <Term>Elastos Improvement Proposal (ELIP) </Term>
          <span>{I18N.get('elip.note')}</span>
        </Text>
      </Wrapper>
    </Col>
  </StyledRow>
)

export default ElipNote

const StyledRow = styled(Row)`
  margin-top: 24px;
  margin-bottom: 24px;
`

const Wrapper = styled.div`
  background: #f6f9fd;
  display: flex;
  padding: 30px 48px 30px 35px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    padding: 24px 20px;
  }
  min-height: 140px;
  justify-content: center;
  align-items: center;
`
const Image = styled.div`
  flex-shrink: 1;
`
const Text = styled.div`
  margin-left: 32px;
  color: #434D72;
  font-size: 14px;
  font-style: italic;
  line-height: 20px;
`
const Term = styled.span`
  color: #000;
`
const StyledCol = styled(Col)`
  margin-right: 20px;
  min-width: 120px;
`
