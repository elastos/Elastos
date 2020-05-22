import React, { Component } from 'react'
import styled from 'styled-components'
import QRCode from 'qrcode.react'
import { Spin, message, Modal } from 'antd'

class WithdrawMoney extends Component {
  constructor(props) {
    super(props)
    this.state = {
      url: ''
    }
  }

  hideModal = () => {
    this.props.hideModal()
  }

  componentDidMount = async () => {
    const { proposalId, withdraw, stage } = this.props
    const rs = await withdraw(proposalId, stage)
    if (rs && !rs.success && rs.url === null) {
      message.info('The business is busy, please try again later.')
      this.setState({ visible: false })
      return
    }
    if (rs && rs.success) {
      this.setState({ url: rs.url })
    }
  }

  render() {
    const { url } = this.state
    return (
      <Modal
        maskClosable={false}
        visible={this.props.withdrawal}
        onCancel={this.hideModal}
        footer={null}
      >
        <Content>
          {url ? <QRCode value={url} size={400} /> : <Spin />}
          <Tip>Scan the QR code above to withdraw ELA.</Tip>
        </Content>
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
