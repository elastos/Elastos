import { breakPoint } from '@/constants/breakPoint'
import { text, bg, primary, border } from '@/constants/color'
import { gap } from '@/constants/variable'
import styled from 'styled-components'
import { Button, Row, Anchor } from 'antd'

export const Container = styled.div`
  max-width: 1200px;
  margin: 32px 32px 80px 360px;
  @media only screen and (max-width: ${breakPoint.xl}) {
    margin-left: 264px;
    margin-right: 5%;
  }
  @media only screen and (max-width: ${breakPoint.lg}) {
    margin-left: 240px;
    margin-right: 32px;
  }
  @media only screen and (max-width: ${breakPoint.mobile}) {
    width: 100%;
    padding: 16px;
    margin: 0;
  }
`

export const Title = styled.div`
  font-size: 30px;
  color: ${text.newGray};
  margin-top: 26px;
  margin-bottom: 8px;
  padding: 4px 8px;
`

export const CoverImg = styled.img`
  width: 100%;
`

export const ShortDesc = styled.div`
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
  color: ${text.darkGray};

  p {
    padding-bottom: 0;
    margin-bottom: 12px;
  }

  ul,
  ol {
    margin-left: 24px;

    > li {
      padding-left: 4px;
    }
  }
`

export const CouncilComments = styled.div`
  padding: 8px;
`

export const BtnGroup = styled.div`
  margin: ${gap.gap_2} 0;
`

export const StyledButton = styled(Button)`
  margin: 8px 0;
  &.cr-btn-ghost:not(:hover) {
    opacity: 0.2;
  }
  > span {
    font-size: 12px !important;
    line-height: 12px !important;
    margin-top: -4px;
  }
`

export const CreateProposalText = styled.div`
  background: #f2f6fb;
  text-align: center;
  padding-top: 18px;
  padding-bottom: 18px;
  font-family: Synthese;
  font-size: 17px;
  line-height: 24px;
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

export const ItemText = styled.span`
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
      content: '';
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

export const Subtitle = styled.div`
  margin: 24px 0;
  color: #000000;
  font-weight: 500;
`
export const Paragraph = styled.div`
  color: rgba(0, 0, 0, 0.75);
  font-variant-ligatures: common-ligatures;
  line-height: 1.8;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
`
