import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin, Divider, message } from 'antd'
import I18N from '@/I18N'
import QRCode from 'qrcode.react'

class LoginWithDid extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: '',
      visible: false
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

  polling = async () => {
    if (!this._isMounted) {
      return
    }
    const { url } = this.state
    const rs = await this.props.checkElaAuth(url)
    if (rs && rs.success === true) {
      clearTimeout(this.timerDid)
      this.timerDid = null
      if (rs.did) {
        this.props.changeTab('register', rs.did)
        this.setState({ visible: false })
      }
      return
    }
    if (rs && rs.success === false) {
      clearTimeout(this.timerDid)
      this.timerDid = null
      if (rs.message) {
        message.error(rs.message)
      } else {
        message.error('Something went wrong')
      }
      this.setState({ visible: false })
      return
    }
    if (this._isMounted) {
      clearTimeout(this.timerDid)
      this.timerDid = setTimeout(this.polling, 3000)
    }
  }

  handleClick = () => {
    if (this.timerDid) {
      return
    }
    this.timerDid = setTimeout(this.polling, 3000)
  }

  componentDidMount = async () => {
    this._isMounted = true
    const rs = await this.props.loginElaUrl()
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  componentWillUnmount() {
    this._isMounted = false
    clearTimeout(this.timerDid)
    this.timerDid = null
  }

  handleVisibleChange = (visible) => {
    this.setState({ visible })
  }

  render() {
    return (
      <Wrapper>
        <Divider>
          <Text>OR</Text>
        </Divider>
        <Popover
          visible={this.state.visible}
          onVisibleChange={this.handleVisibleChange}
          content={this.elaQrCode()}
          trigger="click"
          placement="top"
        >
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
  color: #031e28;
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
