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
      visible: false
    }
  }

  elaQrCode = () => {
    const { url } = this.state
    return (
      <Content>
        {url ? <QRCode value={url} size={400} /> : <Spin />}
        <Tip>Scan the QR code above to make this suggestion into proposal.</Tip>
      </Content>
    )
  }

  proposalState = async () => {
    const { id, pollProposalState, history } = this.props
    const rs = await pollProposalState({ id })
    if (rs && rs.success) {
      if (rs.proposer === false) {
        message.info('This suggestion had been made into proposal by other council member.')
      }
      history.push(`/proposals/${rs.id}`)
    }
    if (rs && rs.success === false) {
      if (rs.message) {
        message.error(rs.message)
      } else {
        message.error('Something went wrong')
      }
      this.setState({ visible: false })
    }
  }

  pollingProposalState = async () => {
    await this.proposalState()
    setTimeout(this.pollingProposalState, 5000)
  }

  handleSign = async () => {
    this.pollingProposalState()
  }

  componentDidMount = async () => {
    const { id, getCMSignatureUrl } = this.props
    const rs = await getCMSignatureUrl(id)
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
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
