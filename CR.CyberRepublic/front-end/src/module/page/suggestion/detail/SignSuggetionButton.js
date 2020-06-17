import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin, message } from 'antd'
import QRCode from 'qrcode.react'
import I18N from '@/I18N'
import { StyledButton } from './style'

class SignSuggestionButton extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: '',
      visible: false,
      message: ''
    }
    this.timer = null
  }

  elaQrCode = () => {
    const { url, message } = this.state
    if (message) {
      return <Content>{message}</Content>
    }
    return (
      <Content>
        {url ? <QRCode value={url} size={400} /> : <Spin />}
        <Tip>{I18N.get('suggestion.msg.signQRCode')}</Tip>
      </Content>
    )
  }

  pollingSignature = async () => {
    if (!this._isMounted) {
      return
    }
    const { id, getSignature } = this.props
    const rs = await getSignature(id)
    if (rs && rs.success) {
      clearTimeout(this.timer)
      this.timer = null
      this.setState({ visible: false })
      return
    }
    if (rs && rs.success === false) {
      clearTimeout(this.timer)
      this.timer = null
      if (rs.message) {
        message.error(rs.message)
      } else {
        message.error(I18N.get('suggestion.msg.exception'))
      }
      this.setState({ visible: false })
      return
    }
    if (this._isMounted === true) {
      clearTimeout(this.timer)
      this.timer = setTimeout(this.pollingSignature, 3000)
    }
  }

  handleSign = async () => {
    if (this.state.message) {
      return
    }
    if (this.timer) {
      return
    }
    this.timer = setTimeout(this.pollingSignature, 3000)
  }

  componentDidMount = async () => {
    this._isMounted = true
    const { id, getSignatureUrl } = this.props
    const rs = await getSignatureUrl(id)
    if (rs && rs.success) {
      this.setState({ url: rs.url, message: '' })
    }
    if (rs && !rs.success) {
      this.setState({ message: rs.message, url: '' })
    }
  }

  componentWillUnmount() {
    this._isMounted = false
    clearTimeout(this.timer)
    this.timer = null
  }

  handleVisibleChange = (visible) => {
    this.setState({ visible })
  }

  render() {
    return (
      <Popover
        content={this.elaQrCode()}
        trigger="click"
        placement="top"
        visible={this.state.visible}
        onVisibleChange={this.handleVisibleChange}
      >
        <StyledButton
          className="cr-btn cr-btn-default"
          onClick={this.handleSign}
        >
          {I18N.get('suggestion.btn.signSuggetion')}
        </StyledButton>
      </Popover>
    )
  }
}

export default SignSuggestionButton

const Content = styled.div`
  padding: 16px;
  text-align: center;
`
const Tip = styled.div`
  font-size: 14px;
  color: #000;
  margin-top: 16px;
`
