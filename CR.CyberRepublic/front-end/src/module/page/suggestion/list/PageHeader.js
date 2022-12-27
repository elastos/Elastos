import React from 'react'
import styled from 'styled-components'
import I18N from '@/I18N'
import SuggestionSvg from './svg/SuggestionSvg'
import CrCouncilSvg from './svg/CrCouncilSvg'
import CrProposalPageSvg from './svg/CrProposalPageSvg'
import ArrowSvg from './svg/ArrowSvg'
import ApprovedProposalSvg from './svg/ApprovedProposalSvg'
import { breakPoint } from '@/constants/breakPoint'

const PageHeader = () => {
  return (
    <Wrapper>
      <First>
        <SuggestionSvg />
        <Text>{I18N.get('suggestion.header.suggestion')}</Text>
      </First>
      <ArrowSvg />
      <Second>
        <CrCouncilSvg />
        <Text>{I18N.get('suggestion.header.crCouncil')}</Text>
      </Second>
      <ArrowSvg />
      <Third>
        <CrProposalPageSvg />
        <Text>{I18N.get('suggestion.header.crProposalPage')}</Text>
      </Third>
      <ArrowSvg />
      <Fourth>
        <ApprovedProposalSvg />
        <Text>{I18N.get('suggestion.header.approvedProposal')}</Text>
      </Fourth>
    </Wrapper>
  )
}

export default PageHeader

const Wrapper = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding-top: 16px;
  margin-bottom: 64px;
  max-width: 960px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    display: none;
  }
  > div {
    text-align: center;
    position: relative;
  }
  > svg {
    flex-shrink: 0;
    margin: 0 8px;
  }
`
const Text = styled.div`
  color: #fff;
  padding-top: 24px;
  text-transform: uppercase;
  position: absolute;
  left: 0;
  right: 0;
  font-weight: 500;
`
const First = styled.div`
  width: 120px;
`
const Second = styled.div`
  width: 120px;
`
const Third = styled.div`
  width: 170px;
`
const Fourth = styled.div`
  width: 170px;
`
