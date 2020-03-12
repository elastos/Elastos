import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin, Divider } from 'antd'
import I18N from '@/I18N'
import QRCode from 'qrcode.react'

class LoginWithDid extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: ''
    }
    this.timerDid = null
  }

  elaQrCode = () => {
    const { url } = this.state
    return (
      <Content>
        {url ? <QRCode value={url} size={180} /> : <Spin />}
        <Tip>{I18N.get('login.qrcodeTip')}</Tip>
      </Content>
    )
  }

  polling = () => {
    const { url } = this.state
    this.timerDid = setInterval(async () => {
      const rs = await this.props.checkElaAuth(url)
      if (rs) {
        clearInterval(this.timerDid)
        this.timerDid = null
        this.setState({ url: '' })
      }
    }, 3000)
  }

  handleClick = async () => {
    if (this.timerDid) {
      return
    }
    this.polling()
  }

  componentDidMount = async () => {
    const rs = await this.props.loginElaUrl()
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  componentWillUnmount() {
    clearInterval(this.timerDid)
  }

  render() {
    return (
      <Wrapper>
        <Divider>
          <Text>OR</Text>
        </Divider>
        <Popover content={this.elaQrCode()} trigger="click" placement="top">
          <Button onClick={this.handleClick}>
            {I18N.get('login.withDid')}
          </Button>
        </Popover>
      </Wrapper>
    )
  }
}

export default LoginWithDid

const Wrapper = styled.div`
  margin-top: 32px;
  text-align: center;
`
const Text = styled.div`
  font-size: 14px;
  color: #031E28;
  opacity: 0.5;
`
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
