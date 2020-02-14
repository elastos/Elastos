import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin } from 'antd'
import I18N from '@/I18N'
import QRCode from 'qrcode.react'

class ProfileDid extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: '',
      did: props.did ? props.did : ''
    }
    this.timerDid = null
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

  handleClick = () => {
    if (this.timerDid || this.state.did) {
      return
    }
    this.timerDid = setInterval(async () => {
      const rs = await this.props.getDid()
      if (rs && rs.success) {
        this.setState({ did: rs.did })
        clearInterval(this.timerDid)
      }
    }, 3000)
  }

  componentDidMount = async () => {
    const rs = await this.props.getElaUrl()
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  componentWillUnmount() {
    clearInterval(this.timerDid)
  }

  render() {
    const { did } = this.state
    if (did) {
      return (
        <Did>
          DID:<span>{did}</span>
        </Did>
      )
    } else {
      return (
        <Popover content={this.elaQrCode()} trigger="click" placement="top">
          <Button onClick={this.handleClick}>
            {I18N.get('profile.associateDid')}
          </Button>
        </Popover>
      )
    }
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
const Did = styled.div`
  line-height: 32px;
  span {
    color: #008d85;
    font-size: 13px;
    padding-left: 10px;
  }
`
