import React, { Component } from 'react'
import styled from 'styled-components'
import { Form, Button, Input, message } from 'antd'
import QRCode from 'qrcode.react'

const { TextArea } = Input
const FormItem = Form.Item

class Signature extends Component {
  constructor(props) {
    super(props)
    this.state = { url: '', messageHash: '' }
    this.timerDid = null
  }

  handleSubmit = (e) => {
    e.stopPropagation() // prevent event bubbling
    e.preventDefault()
    const { form, applyPayment, proposalId, stage } = this.props
    form.validateFields(async (err, values) => {
      if (!err) {
        const rs = await applyPayment(proposalId, stage, values)
        if (rs.success && rs.url) {
          this.setState({ url: rs.url, messageHash: rs.messageHash })
          this.pollingSignature()
        }
      }
    })
  }

  pollingSignature = () => {
    const { proposalId, getPaymentSignature, hideModal } = this.props
    const { messageHash } = this.state
    this.timerDid = setInterval(async () => {
      const rs = await getPaymentSignature({ proposalId, messageHash })
      if (rs && rs.success) {
        clearInterval(this.timerDid)
        this.timerDid = null
        hideModal()
      }
      if (rs && rs.success === false) {
        clearInterval(this.timerDid)
        this.timerDid = null
        if (rs.message) {
          message.error(rs.message)
        } else {
          message.error('Something went wrong')
        }
        hideModal()
      }
    }, 5000)
  }

  componentWillUnmount() {
    clearInterval(this.timerDid)
  }

  signatureQrCode = () => {
    const { url } = this.state
    return (
      <Content>
        {url ? <QRCode value={url} size={400} /> : <Spin />}
        <Tip>Scan the QR code above to sign your application.</Tip>
      </Content>
    )
  }

  renderTextare = () => {
    const { getFieldDecorator } = this.props.form
    const { isSecretary } = this.props
    return (
      <Form onSubmit={this.handleSubmit}>
        {isSecretary && <div>application</div>}
        <Label>
          <span>*</span>
          Reason
        </Label>
        <FormItem>
          {getFieldDecorator('message', {
            rules: [
              {
                required: true,
                message: 'This field is required'
              }
            ]
          })(<TextArea rows={16} style={{ resize: 'none' }} />)}
        </FormItem>
        <Actions>
          <Button className="cr-btn cr-btn-primary" htmlType="submit">
            Next
          </Button>
        </Actions>
      </Form>
    )
  }

  render() {
    const { url } = this.state
    const { stage, isSecretary } = this.props
    return (
      <Wrapper>
        {isSecretary ? (
          <Title>Review Payment #{parseInt(stage) + 1}</Title>
        ) : (
          <Title>Apply Payment #{parseInt(stage) + 1}</Title>
        )}

        {url ? this.signatureQrCode() : this.renderTextare()}
      </Wrapper>
    )
  }
}

export default Form.create()(Signature)

const Wrapper = styled.div`
  margin: 0 auto;
`
const Label = styled.div`
  font-size: 17px;
  color: #000;
  display: block;
  margin-bottom: 8px;
  > span {
    color: #ff0000;
  }
`
const Title = styled.div`
  font-size: 30px;
  line-height: 42px;
  color: #000000;
  text-align: center;
  margin-bottom: 42px;
`
const Actions = styled.div`
  display: flex;
  justify-content: center;
  > button {
    margin: 0 8px;
  }
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
