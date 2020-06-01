import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin, message } from 'antd'
import QRCode from 'qrcode.react'
import I18N from '@/i18n'
import { StyledButton } from './style'

class SignSuggestionButton extends Component {
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
        {url ? <QRCode value={url} size={400} /> : <Spin />}
        <Tip>{I18N.get('suggestion.sign')}</Tip>
      </Content>
    )
  }

  pollingSignature = () => {
    const { id, getSignature } = this.props
    this.timerDid = setInterval(async () => {
      const rs = await getSignature(id)
      if (rs && rs.success) {
        clearInterval(this.timerDid)
        this.timerDid = null
        this.setState({ url: '', visible: false })
      }
      if (rs && rs.success === false) {
        clearInterval(this.timerDid)
        this.timerDid = null
        if (rs.message) {
          message.error(rs.message)
        } else {
          message.error(I18N.get('suggestion.error.exception'))
        }
        this.setState({ visible: false })
      }
    }, 3000)
  }

  handleSign = async () => {
    if (this.timerDid) {
      return
    }
    this.pollingSignature()
  }

  componentDidMount = async () => {
    const { id, getSignatureUrl } = this.props
    const rs = await getSignatureUrl(id)
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  componentWillUnmount() {
    clearInterval(this.timerDid)
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
          {I18N.get('suggestion.signBtn')}
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
