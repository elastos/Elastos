import { breakPoint } from '@/constants/breakPoint'
import { text, bg } from '@/constants/color'
import { gap } from '@/constants/variable'
import styled from 'styled-components'
import { Button } from 'antd'

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

export const Title = styled.div`
  font-size: 30px;
  color: ${text.primaryDark};
  margin-top: 26px;
  margin-bottom: 8px;
 `

export const ShortDesc = styled.div`
  font-weight: 200;
  color: ${text.darkGray};
  padding: 12px 8px 0;
`


export const Label = styled.span`
  background: ${bg.blue};
  margin-right: 10px;
  word-break: keep-all;
  
  padding: 4px 8px;
  color: ${text.newGray};
  border: 1px solid #e4effd;
  border-radius: 4px;
  
`

export const LabelPointer = styled(Label)`
  cursor: pointer;
`

export const DescLabel = styled.h4`
  color: ${text.newGray};
`

export const Desc = styled.div`
  color: black;
  margin-top: 8px;
`

export const DescBody = styled.div`
  font-weight: 200;
  color: ${text.darkGray};
  
  p {
    padding-bottom: 0;
    margin-bottom: 12px;
  }
`

export const CouncilComments = styled.div`
  padding: 8px;
  font-weight: 200;
`

// export const StyledLink = styled.div`
//   color: ${text.lightGray};
//   margin-top: ${gap.gap_2};
// `

export const BtnGroup = styled.div`
  margin: ${gap.gap_2} 0;
`

export const StyledButton = styled(Button)`
  width: 200px;
  height: 44px !important;
  
  > span {
    font-size: 12px !important;
    line-height: 12px !important;
    display: block;
    margin-top: -4px;
  }
`
