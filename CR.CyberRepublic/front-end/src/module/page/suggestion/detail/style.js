import { breakPoint } from '@/constants/breakPoint'
import { text, bg, primary, border } from '@/constants/color'
import { gap } from '@/constants/variable'
import styled from 'styled-components'
import { Button, Row, Anchor } from 'antd'

export const Container = styled.div`
  max-width: 1200px;
  margin: 30px 30px 80px 290px;

  @media only screen and (max-width: ${breakPoint.xl}) {
    margin-left: 210px;
    margin-right: 5%;
  }

  @media only screen and (max-width: ${breakPoint.lg}) {
    margin-left: 210px;
  }
  @media only screen and (max-width: ${breakPoint.mobile}) {

    width: 90vw;
  }
`

export const Title = styled.div`
  font-size: 30px;
  color: ${text.newGray};
  margin-top: 26px;
  margin-bottom: 8px;
  background-color: ${bg.blue};
  border: 1px solid ${border.lightBlue};
  /* border-radius: 4px; */
  padding: 4px 8px;
 `

export const CoverImg = styled.img`
  width: 100%;
`

export const ShortDesc = styled.div`
  /* font-weight: 200; */
  color: ${text.darkGray};
  padding: 12px 8px 0;
`


export const Label = styled.span`
  background: ${bg.blue};
  margin-right: 10px;
  word-break: keep-all;

  padding: 4px 8px;
  color: ${text.newGray};
  border: 1px solid ${border.lightBlue};
  /* border-radius: 4px; */

`

export const LabelPointer = styled(Label)`
  cursor: pointer;
  display: inline;
  &:hover {
    background-color: ${primary.light};
  }
`

export const DescLabel = styled.h4`
  color: ${text.newGray};
`

export const Desc = styled.div`
  color: black;
  margin-top: 8px;
`

export const DescBody = styled.div`
  /* font-weight: 200; */
  color: ${text.darkGray};

  p {
    padding-bottom: 0;
    margin-bottom: 12px;
  }

  ul, ol {
    margin-left: 24px;

    > li {
      padding-left: 4px;
      /* font-weight: 200; */
    }
  }
`

export const CouncilComments = styled.div`
  padding: 8px;
  /* font-weight: 200; */
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

export const IconWrap = styled.div`
  display: inline-block;
  position: relative;
  top: 4px;
`

export const Item = styled(Row)`
  margin-top: 10px;
  font-size: 13px;
  font-style: italic;
`
export const ItemTitle = styled.div`
  font-weight: 400;
  :after {
    content: ':';
  }
`
export const ItemText = styled.div`
  font-weight: 200;
`

export const StyledAnchor = styled(Anchor)`
  position: fixed;
  top: 250px;
  left: 30px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    display: none;
  }
  .ant-anchor-ink:before {
    width: 0;
  }
  .ant-anchor-ink-ball.visible {
    display: none;
  }
  .ant-anchor-link-title {
    display: inline;
  }
  .ant-anchor-link-active > .ant-anchor-link-title {
    color: initial;
    :after {
      content: "";
      position: absolute;
      bottom: -2px;
      left: 0;
      right: 0;
      height: 0.5em;
      border-bottom: 8px solid ${text.green};
      z-index: -1;
    }
  }
`

export const StyledRichContent = styled.div`
  .md-RichEditor-root {
    padding: 0;
    figure.md-block-image {
      background: none;
    }
    figure.md-block-image figcaption .public-DraftStyleDefault-block {
      text-align: left;
    }
  }
`
