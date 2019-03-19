import { breakPoint } from '@/constants/breakPoint'
import { text } from '@/constants/color'
import { gap } from '@/constants/variable'
import styled from 'styled-components'

export const Container = styled.div`
  padding: 0 50px 80px;
  width: 80vw;
  margin: 80px auto 0;

  @media only screen and (max-width: ${breakPoint.lg}) {
    margin-left: 5%;
    margin-right: 5%;
  }
  @media only screen and (max-width: ${breakPoint.mobile}) {
    padding: 0;
    width: 90vw;
  }
`

export const Title = styled.div`
  font-size: 30px;
  color: ${text.primaryDark};
  margin-top: 26px;
  margin-bottom: 18px;
  `

export const Desc = styled.div`
  color: black;
`

export const Link = styled.div`
  color: ${text.lightGray};
  margin-top: ${gap.gap_2};
  `
// export const Actions = styled.div`
//   margin-top: 27px;
//   margin-bottom: 50px;
// `
