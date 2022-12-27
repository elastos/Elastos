import styled from 'styled-components'
import { breakPoint } from '@/constants/breakPoint'
import { Container as PContainer } from '../detail/style'

export const Container = styled(PContainer)`
`

export const Header = styled.h2`
  font-family: Synthese;
  font-style: normal;
  font-weight: normal;
  font-size: 20px;
  line-height: 28px;
  
  padding-top: 0;
  @media only screen and (max-width: ${breakPoint.mobile}) {
  padding-top: 30px;
  }
`

export const List = styled.div`
  margin-bottom: 40px;
`

export const Item = styled.div`
  padding: 30px 0;
  &:not(:last-child) {
  border-bottom: 1px solid #E5E5E5;
  }
`

export const Title = styled.h4`
  font-size: 20px;
  color: rgba(0, 0, 0, 0.65);
  margin-top: 8px;
  margin-bottom: 0;
  display: block;
`
