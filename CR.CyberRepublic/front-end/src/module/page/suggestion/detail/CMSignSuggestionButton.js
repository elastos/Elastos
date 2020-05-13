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
        {url ? <QRCode value={url} size={600} /> : <Spin />}
        <Tip>Scan the QR code above to make this suggestion into proposal.</Tip>
      </Content>
    )
  }

  pollingProposalState = () => {
    const { id, pollProposalState, history } = this.props
    this.timerDid = setInterval(async () => {
      const rs = await pollProposalState(id)
      if (rs && rs.success) {
        clearInterval(this.timerDid)
        this.timerDid = null
        history.push(`/proposals/${rs.id}`)
      }
      if (rs && rs.success === false) {
        clearInterval(this.timerDid)
        this.timerDid = null
        if (rs.message) {
          message.error(rs.message)
        } else {
          message.error('Something went wrong')
        }
        this.setState({ visible: false })
      }
    }, 3000)
  }

  handleSign = async () => {
    if (this.timerDid) {
      return
    }
    this.pollingProposalState()
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
