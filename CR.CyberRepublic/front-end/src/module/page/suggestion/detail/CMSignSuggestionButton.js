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
      loading: false
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
    const { url } = this.state
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
    //clear timer
    this.clearTimerList()
    await this.sleep(5000)
    this.pollingProposalState()
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

  handleVisibleChange = (visible) => {
    this.setState({ visible })
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
      <Popover
        content={this.elaQrCode()}
        trigger="click"
        placement="top"
        visible={visible}
        onVisibleChange={this.handleVisibleChange}
      >
        <StyledButton
          type="ebp"
          className="cr-btn cr-btn-default"
          onClick={this.handleSign}
        >
          {I18N.get('suggestion.btn.makeIntoProposal')}
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
