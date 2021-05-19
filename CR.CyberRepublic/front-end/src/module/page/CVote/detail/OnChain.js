import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin } from 'antd'
import I18N from '@/I18N'
import QRCode from 'qrcode.react'

class OnChainButton extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: '',
      visible: false
    }
  }

  qrCode = () => {
    const { url } = this.state
    return (
      <Content>
        {url ? <QRCode value={url} size={300} /> : <Spin />}
        <Tip>{I18N.get('profile.qrcodeTip')}</Tip>
      </Content>
    )
  }

  componentDidMount = async () => {
    const { id, getReviewProposalUrl } = this.props
    const rs = await getReviewProposalUrl(id)
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  handleVisibleChange = (visible) => {
    this.setState({ visible })
  }

  render() {
    let domain
    if (process.env.NODE_ENV === 'development') {
      domain = 'blockchain-did-regtest'
    } else {
      domain = 'idchain'
    }
    return (
      <Popover
        content={this.qrCode()}
        trigger="click"
        placement="top"
        visible={this.state.visible}
        onVisibleChange={this.handleVisibleChange}
      >
        <Button>{I18N.get('council.voting.voteResult.onchain')}</Button>
      </Popover>
    )
  }
}

export default OnChainButton

const Button = styled.span`
  display: inline-block;
  margin-bottom: 16px;
  font-size: 13px;
  border: 1px solid #008d85;
  color: #008d85;
  text-align: center;
  padding: 6px 16px;
  cursor: pointer;
`
const Content = styled.div`
  padding: 16px;
  text-align: center;
`
const Tip = styled.div`
  font-size: 14px;
  color: #000;
  margin-top: 16px;
`
