import React from 'react'
import { Modal } from 'antd'
import {
  FacebookShareButton,
  TwitterShareButton,
  RedditShareButton,
  WeiboShareButton,
  FacebookIcon,
  TwitterIcon,
  RedditIcon
} from 'react-share'
import styled from 'styled-components'
import QRCode from 'qrcode.react'
import I18N from '@/I18N'

import BaseComponent from '@/model/BaseComponent'
import WeiboIcon from './WeiboIcon'
import WeChatIcon from './WeChatIcon'

class SocialShareButtons extends BaseComponent {
  ord_states() {
    return {
      toggle: false
    }
  }

  toggleWechat = () => {
    this.setState({ toggle: !this.state.toggle })
  }

  ord_render() {
    const { shareQuote } = this.props
    return (
      <Wrapper>
        <FacebookShareButton url={window.location.href} quote={shareQuote}>
          <FacebookIcon size={32} round={true} />
        </FacebookShareButton>
        <TwitterShareButton url={window.location.href} title={shareQuote}>
          <TwitterIcon size={32} round={true} />
        </TwitterShareButton>
        <RedditShareButton
          url={window.location.href}
          title={shareQuote}
          windowWidth={660}
          windowHeight={460}
        >
          <RedditIcon size={32} round={true} />
        </RedditShareButton>
        <WeiboShareButton url={window.location.href} title={shareQuote}>
          <WeiboIcon size={32} />
        </WeiboShareButton>
        <WeChatShareButton onClick={this.toggleWechat} className="wechat">
          <WeChatIcon size={32} />
          <Modal visible={this.state.toggle} footer={null}>
            <InnerWrapper>
              <Desc>{I18N.get('socialShare.wechat.desc')}</Desc>
              <QRCode value={window.location.href} size={256} />
            </InnerWrapper>
          </Modal>
        </WeChatShareButton>
      </Wrapper>
    )
  }
}

export default SocialShareButtons

const Wrapper = styled.div`
  display: flex;
  margin-top: 8px;
  div {
    margin-right: 4px;
    cursor: pointer;
  }
  .wechat {
    @media (max-width: 768px) {
      display: none;
    }
  }
`
const WeChatShareButton = styled.div`
  margin-left: 4px;
`
const Desc = styled.div`
  font-size: 16px;
  margin-bottom: 16px;
`
const InnerWrapper = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;
`
