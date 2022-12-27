import React, { Component } from 'react'
import styled from 'styled-components'
import QRCode from 'qrcode.react'
import { Modal } from 'antd'
import I18N from '@/I18N'

class WithdrawMoney extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: '',
      message: ''
    }
  }

  hideModal = () => {
    this.props.hideModal()
  }

  componentDidMount = async () => {
    const { proposalId, withdraw, stage } = this.props
    const rs = await withdraw(proposalId, stage)
    if (rs && !rs.success && rs.url === null) {
      this.setState({
        message: I18N.get('milestone.noUtxos')
      })
    }
    if (rs && rs.success) {
      this.setState({ url: rs.url, message: '' })
    }
  }

  render() {
    const { url, message } = this.state
    return (
      <Modal
        maskClosable={false}
        visible={this.props.withdrawal}
        onCancel={this.hideModal}
        footer={null}
      >
        {url ? (
          <Content>
            <QRCode value={url} size={400} />
            <Tip>{I18N.get('milestone.scanToWithdraw')}</Tip>
          </Content>
        ) : (
          <Content>{message}</Content>
        )}
      </Modal>
    )
  }
}

export default WithdrawMoney

const Content = styled.div`
  padding: 16px;
  text-align: center;
`
const Tip = styled.div`
  font-size: 14px;
  color: #000;
  margin-top: 16px;
`
