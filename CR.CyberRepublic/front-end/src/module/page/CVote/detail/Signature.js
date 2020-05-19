import React, { Component } from 'react'
import styled from 'styled-components'
import { Form, Button, Input } from 'antd'
import QRCode from 'qrcode.react'

const { TextArea } = Input
const FormItem = Form.Item

class Signature extends Component {
  constructor(props) {
    super(props)
    this.state = { url: '' }
  }

  handleSubmit = (e) => {
    e.stopPropagation() // prevent event bubbling
    e.preventDefault()
    const { form, applyPayment, proposalId, stage } = this.props
    form.validateFields(async (err, values) => {
      if (!err) {
        const rs = await applyPayment(proposalId, stage, values)
        if (rs.success && rs.url) {
          this.setState({ url: rs.url })
        }
      }
    })
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
    return (
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
    )
  }

  render() {
    const { url } = this.state
    return (
      <Wrapper>
        <Title>Apply Payment</Title>
        <Form onSubmit={this.handleSubmit}>
          <Label>
            <span>*</span>
            Reason
          </Label>
          {url ? this.signatureQrCode() : this.renderTextare()}
          <Actions>
            <Button
              className="cr-btn cr-btn-default"
              onClick={() => {
                this.props.onCancel()
              }}
            >
              Cancel
            </Button>
            <Button className="cr-btn cr-btn-primary" htmlType="submit">
              Next
            </Button>
          </Actions>
        </Form>
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
