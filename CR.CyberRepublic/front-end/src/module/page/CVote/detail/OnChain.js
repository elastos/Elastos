import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin } from 'antd'
import I18N from '@/I18N'
import QRCode from 'qrcode.react'
import { supportsHistory } from 'history/DOMUtils'

class ProfileDid extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: '',
      id: '',
      visible: false
    }
    this.timerDid = null
  }

  qrCode = () => {
    const { url } = this.state
    return (
      <Content>
        {url ? <QRCode value={url} size={145} /> : <Spin />}
        <Tip>{I18N.get('profile.qrcodeTip')}</Tip>
      </Content>
    )
  }

  pollingDid = () => {
    const voteId = this.props._id
    this.setState({id:voteId})
    this.timerDid = setInterval(async () => {
      const rs = await this.props.getReviewProposal(this.state.id)
      this.setState({ url: rs.url })
    }, 3000)
  }

  handleAssociate = async () => {
    this.pollingDid()
  }

  componentDidMount = async () => {
    const { id } = this.state
    const rs = await this.props.getReviewProposal(id)
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  handleVisibleChange = visible => {
    this.setState({ visible })
  }

  render() {
    let domain
    if (process.env.NODE_ENV === 'development') {
      domain = 'blockchain-did-regtest'
    } else {
      domain = 'idchain'
    }
    return (
        <Popover content={this.qrCode()} trigger="click" placement="right">
            <Button onClick={this.handleAssociate}>
                {I18N.get('council.voting.voteResult.onchain')}
            </Button>
        </Popover>
    )
  }
}

export default ProfileDid

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
const Did = styled.div`
  line-height: 32px;
  a {
    color: #008d85;
    font-size: 13px;
    padding-left: 10px;
    &:focus {
      text-decoration: none;
    }
  }
`
const Reassociate = styled.span`
  display: inline-block;
  font-size: 13px;
  color: #008d85;
  cursor: pointer;
  border: 1px solid #008d85;
  padding: 0 8px;
  margin-bottom: 16px;
`
