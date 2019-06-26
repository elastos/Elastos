import styled from 'styled-components'
import { Anchor } from 'antd'
// import { breakPoint } from '@/constants/breakPoint'
import { text } from '@/constants/color'
// import { gap } from '@/constants/variable'

export const Label = styled.span`
  background: #F2F6FB;
  padding: 3px 10px;
`

export const Title = styled.h2`
  padding-bottom: 15px;
`

export const ContentTitle = styled.h4`
  font-size: 20px;
  padding-bottom: 0;
`

export const FixedHeader = styled.div`
  position: fixed;
  padding-top: 80px;
  background: white;
  z-index: 1000;
  width: 100%;
`

export const Body = styled.div`
  padding-top: 270px;
`

export const StyledAnchor = styled(Anchor)`
  position: fixed;
  top: 250px;
  left: 30px;
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
