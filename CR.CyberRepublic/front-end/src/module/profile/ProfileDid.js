import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin } from 'antd'
import I18N from '@/I18N'
import QRCode from 'qrcode.react'

class ProfileDid extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: ''
    }
  }

  elaQrCode = () => {
    const { url } = this.state
    return (
      <Content>
        {url ? <QRCode value={url} size={145} /> : <Spin />}
        <Tip>{I18N.get('profile.qrcodeTip')}</Tip>
      </Content>
    )
  }

  componentDidMount = async () => {
    const rs = await this.props.getElaUrl()
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  render() {
    return (
      <Popover
        content={this.elaQrCode()}
        trigger="click"
        placement="top"
      >
        <Button>{I18N.get('profile.associateDid')}</Button>
      </Popover>
    )
  }
}

export default ProfileDid

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
