import React, { Component } from 'react'
import styled from 'styled-components'
import { Modal, Button, Spin } from 'antd'
import I18N from '@/I18N'
import QRCode from 'qrcode.react'

class SignSuggestionModal extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: '',
      visible: true,
      message: '',
      signNow: false
    }
    this.timer = null
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
    this.setState({ signNow: true })
    const { id, getSignatureUrl } = this.props
    const rs = await getSignatureUrl(id)
    if (rs && rs.success) {
      this.setState({ url: rs.url })
      this.timer = setTimeout(this.pollingSignature, 3000)
    }
    if (rs && rs.success === false && rs.message) {
      this.setState({ message: rs.message })
    }
  }

  componentDidMount = () => {
    this._isMounted = true
  }

  componentWillUnmount = () => {
    this._isMounted = false
    clearTimeout(this.timer)
    this.timer = null
  }

  elaQrCode = () => {
    const { url } = this.state
    return (
      <Content>
        {url ? <QRCode value={url} size={400} /> : <Spin />}
        <Tip>{I18N.get('suggestion.msg.signQRCode')}</Tip>
      </Content>
    )
  }

  modalContent = () => {
    const { message, signNow } = this.state
    if (message) {
      return <Content>{message}</Content>
    }
    if (signNow) {
      return <Content>{this.elaQrCode()}</Content>
    }
    return (
      <Content>
        <Notice>{I18N.get('suggestion.modal.signNotice')}</Notice>
        <Button
          className="cr-btn cr-btn-default"
          onClick={() => {
            this.setState({ visible: false })
          }}
        >
          {I18N.get('suggestion.modal.signLater')}
        </Button>
        <Button
          style={{ marginLeft: 24 }}
          className="cr-btn cr-btn-primary"
          onClick={this.handleSign}
        >
          {I18N.get('suggestion.modal.signNow')}
        </Button>
      </Content>
    )
  }

  hideModal = () => {
    this.setState({ visible: false })
  }

  render() {
    const { visible } = this.state
    return (
      <Modal
        maskClosable={false}
        visible={visible}
        onCancel={this.hideModal}
        footer={null}
        width={500}
      >
        {this.modalContent()}
      </Modal>
    )
  }
}

export default SignSuggestionModal

const Content = styled.div`
  padding: 8px 0;
  text-align: center;
`
const Tip = styled.div`
  font-size: 14px;
  color: #000;
  margin-top: 16px;
`
const Notice = styled.div`
  font-size: 16px;
  color: #000;
  margin-bottom: 24px;
`
