import React, { Component } from 'react'
import styled from 'styled-components'
import { Modal, Spin, Button } from 'antd'
import QRCode from 'qrcode.react'
import I18N from '@/I18N'
import { StyledButton } from './style'

class CMSignSuggestionButton extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: '',
      visible: false,
      loading: false,
      isBound: false
    }
  }

  elaQrCode = () => {
    const { url, isBound, message } = this.state
    if (!isBound) {
      return (
        <Content>
          <Notice>{I18N.get('suggestion.msg.associateDidFirst')}</Notice>
          <Button
            className="cr-btn cr-btn-primary"
            onClick={() => {
              this.props.history.push('/profile/info')
            }}
          >
            {I18N.get('suggestion.btn.associateDid')}
          </Button>
        </Content>
      )
    }
    if (isBound && message) {
      return <Content>{message}</Content>
    }
    return (
      <Content>
        {url ? <QRCode value={url} size={400} /> : <Spin />}
        <Tip>{I18N.get('suggestion.msg.councilQRCode')}</Tip>
      </Content>
    )
  }

  handleSign = async () => {
    const { user } = this.props
    if (user && user.did && user.did.id) {
      this.setState({ isBound: true, visible: true })
    } else {
      this.setState({ isBound: false, visible: true })
    }
  }

  componentDidMount = async () => {
    const { id, getCMSignatureUrl, user } = this.props
    if (user && user.did && user.did.id) {
      const rs = await getCMSignatureUrl(id)
      if (rs && rs.success) {
        this.setState({ url: rs.url, message: '' })
      }
      if (rs && !rs.success && rs.message) {
        this.setState({ message: rs.message, url: '' })
      }
    }
  }

  hideModal = () => {
    this.setState({ visible: false })
  }

  render() {
    const { visible } = this.state
    return (
      <div>
        <StyledButton
          type="ebp"
          className="cr-btn cr-btn-default"
          onClick={this.handleSign}
        >
          {I18N.get('suggestion.btn.makeIntoProposal')}
        </StyledButton>
        <Modal
          maskClosable={false}
          visible={visible}
          onCancel={this.hideModal}
          footer={null}
          width={500}
        >
          {this.elaQrCode()}
        </Modal>
      </div>
    )
  }
}

export default CMSignSuggestionButton

const Content = styled.div`
  padding: 16px;
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
