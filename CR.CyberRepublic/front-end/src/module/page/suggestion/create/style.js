import { breakPoint } from '@/constants/breakPoint'
import styled from 'styled-components'

export const Container = styled.div`
  max-width: 1200px;
  margin: 80px auto;

  @media only screen and (max-width: ${breakPoint.xl}) {
    margin-left: 108px;
    margin-right: 5%;
  }
  @media only screen and (max-width: ${breakPoint.lg}) {
    margin-left: 5%;
  }
  @media only screen and (max-width: ${breakPoint.mobile}) {
    width: 90vw;
  }
`
