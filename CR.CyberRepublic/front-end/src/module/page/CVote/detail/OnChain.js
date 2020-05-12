import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin } from 'antd'
import I18N from '@/I18N'
import QRCode from 'qrcode.react'

class OnChainButton extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: '',
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
    console.log(this.props)
    const { id, getReviewProposal } = this.propos
    this.timerDid = setInterval(async () => {
      const rs = await getReviewProposal(id)
      if (rs && rs.success ){
        clearInterval(this.timerDid)
        this.timerDid = null
        this.setState({ url: rs.url, visible:false })
      }
      if (rs && rs.success == false){
        clearInterval(this.timerDid)
        this.timerDid = null
        if(rs.message) {
          message.error(rs.message)
        } else {
          message.error('Something went wrong')
        }
        this.setState({ visible: false})
      }
    }, 3000)
  }

  handleAssociate = async () => {
    if (this.timerDid){
      return
    }
    this.pollingDid()
  }

  componentDidMount = async () => {
    const { id, getReviewProposalUrl } = this.propos
    console.log(id)
    const rs = await getReviewProposalUrl(id)
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  componentWillUnmount() {
    clearInterval(this.timerDid)
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
        <Popover 
        content={this.qrCode()} 
        trigger="click" 
        placement="top"
        visible={this.state.visible}
        onVisibleChange={this.handleVisibleChange}
        >
            <Button onClick={this.handleAssociate}>
                {I18N.get('council.voting.voteResult.onchain')}
            </Button>
        </Popover>
    )
  }
}

export default OnChainButton

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
