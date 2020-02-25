import React, { Component } from 'react'
import styled from 'styled-components'
import { Popover, Spin } from 'antd'
import I18N from '@/I18N'
import QRCode from 'qrcode.react'
import ExternalLinkSvg from './ExternalLinkSvg'

class ProfileDid extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: ''
    }
    this.timerDid = null
  }

  elaQrCode = () => {
    const { url } = this.state
    return (
      <Content>
        {url ? <QRCode value={url} size={145} /> : <Spin />}
        <Tip>{I18N.get('profile.qrcodeTip')}</Tip>
      </Content>
    )
  }

  pollingDid = () => {
    this.timerDid = setInterval(async () => {
      const rs = await this.props.getNewActiveDid()
      if (rs && rs.success) {
        clearInterval(this.timerDid)
        this.timerDid = null
        this.setState({ url: '' })
      }
    }, 3000)
  }

  handleReassociate = async () => {
    const { url } = this.state
    if (url) {
      if (this.timerDid) {
        return
      } else {
        this.pollingDid()
      }
    } else {
      const rs = await this.props.getElaUrl()
      if (rs && rs.success) {
        this.setState({ url: rs.url }, () => {
          this.pollingDid()
        })
      }
    }
  }

  handleAssociate = async () => {
    if (this.timerDid) {
      return
    }
    this.pollingDid()
  }

  componentDidMount = async () => {
    const rs = await this.props.getElaUrl()
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  componentWillUnmount() {
    clearInterval(this.timerDid)
  }

  render() {
    const { did } = this.props
    let domain
    if (process.env.NODE_ENV === 'production') {
      domain = 'blockchain-did-mainnet'
    } else {
      domain = 'blockchain-did-testnet'
    }
    if (did && did.id) {
      return (
        <Did>
          <span>DID:</span>
          <a
            href={`https://${domain}.elastos.org/address/${did.id}`}
            target="_blank"
          >
            {did.id} <ExternalLinkSvg />
          </a>
          <Popover content={this.elaQrCode()} trigger="click" placement="top">
            <Reassociate onClick={this.handleReassociate}>
              {I18N.get('profile.reassociateDid')}
            </Reassociate>
          </Popover>
        </Did>
      )
    } else {
      return (
        <Popover content={this.elaQrCode()} trigger="click" placement="top">
          <Button onClick={this.handleAssociate}>
            {I18N.get('profile.associateDid')}
          </Button>
        </Popover>
      )
    }
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
  font-size: 13px;
  color: #008d85;
  padding-left: 32px;
  cursor: pointer;
`
