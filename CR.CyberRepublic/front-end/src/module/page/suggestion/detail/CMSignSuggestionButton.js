import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin, message, Tooltip } from 'antd'
import QRCode from 'qrcode.react'
import I18N from '@/I18N'
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
        {url ? <QRCode value={url} size={145} /> : <Spin />}
        <Tip>Scan the QR code above to make this suggestion into proposal.</Tip>
      </Content>
    )
  }

  makeIntoProposal = async () => {
    const { id, proposeSuggestion, history } = this.props
    const param = { suggestionId: id }
    const res = await proposeSuggestion(param)
    history.push(`/proposals/${res._id}`)
  }

  pollingProposalState = () => {
    this.timerDid = setInterval(async () => {
      // polling ela node rpc
      // make into proposal if proposal's state on chain is Registered
      await this.makeIntoProposal()
    }, 3000)
  }

  handleSign = async () => {
    if (this.timerDid) {
      return
    }
    // this.pollingProposalState()
  }

  componentDidMount = async () => {
    const { id, getCMSignatureUrl } = this.props
    const rs = await getCMSignatureUrl(id)
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
