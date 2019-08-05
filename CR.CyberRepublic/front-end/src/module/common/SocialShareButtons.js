import React from 'react'
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

import WeiboIcon from './WeiboIcon'

const SocialShareButtons = ({ shareQuote }) => {
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
        <WeiboIcon size={31} />
      </WeiboShareButton>
    </Wrapper>
  )
}

export default SocialShareButtons

const Wrapper = styled.div`
  display: flex;
  margin-top: 8px;
  div {
    margin-right: 4px;
    cursor: pointer;
  }
`
