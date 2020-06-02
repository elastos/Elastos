import React, { Component } from 'react'
import styled from 'styled-components'
import { Modal, Spin, message, Button } from 'antd'
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
    this.timerList = []
  }

  sleep = (ms) => {
    return new Promise((resolve) => setTimeout(resolve, ms))
  }

  clearTimerList = () => {
    if (this.timerList && this.timerList.length) {
      for (let timer of this.timerList) {
        clearTimeout(timer)
      }
      this.timerList = []
    }
  }

  elaQrCode = () => {
    const { url, isBound } = this.state
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
    return (
      <Content>
        {url ? <QRCode value={url} size={400} /> : <Spin />}
        <Tip>{I18N.get('suggestion.msg.councilQRCode')}</Tip>
      </Content>
    )
  }

  pollingProposalState = async () => {
    const { id, pollProposalState } = this.props
    const rs = await pollProposalState({ id })
    if (rs && rs.success && rs.toChain) {
      this.setState({ visible: false, loading: true })
    }
    if (rs && rs.success && rs.reference) {
      this.clearTimerList()
      this.setState({ visible: false })
      return
    }
    if (rs && rs.success === false) {
      this.clearTimerList()
      if (rs.proposer === false) {
        return
      }
      if (rs.message) {
        message.error(rs.message)
      } else {
        message.error(I18N.get('suggestion.error.exception'))
      }
      this.setState({ visible: false })
      return
    }
    const timer = setTimeout(this.pollingProposalState, 5000)
    this.timerList.push(timer)
  }

  handleSign = async () => {
    const { user } = this.props
    if (user && user.did && user.did.id) {
      this.setState({ isBound: true, visible: true })
      //clear timer
      this.clearTimerList()
      await this.sleep(5000)
      this.pollingProposalState()
    } else {
      this.setState({ isBound: false, visible: true })
    }
  }

  componentDidMount = async () => {
    const { id, getCMSignatureUrl, isProposed, curProposer } = this.props
    if (isProposed && curProposer) {
      this.pollingProposalState()
      return
    }
    const rs = await getCMSignatureUrl(id)
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  componentWillUnmount = async () => {
    this.clearTimerList()
  }

  hideModal = () => {
    this.setState({ visible: false })
  }

  render() {
    const { visible, loading } = this.state
    const { isProposed } = this.props
    if (isProposed && isProposed === true) {
      return (
        <StyledButton
          type="ebp"
          className="cr-btn cr-btn-primary"
          disabled
          loading={loading}
        >
          {I18N.get('suggestion.msg.toChain')}
        </StyledButton>
      )
    }
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
