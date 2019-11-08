import React, { Component } from 'react'
import styled from 'styled-components'
import { Button, Modal, Form, Input } from 'antd'
import I18N from '@/I18N'

const FormItem = Form.Item

class PaymentSchedule extends Component {
  constructor(props) {
    super(props)
    this.state = {
      visible: false,
      paymentItems: []
    }
  }

  hideModal = () => {
    this.setState({ visible: false })
  }

  showModal = () => {
    this.setState({ visible: true })
  }

  handleSubmit = e => {
    e.stopPropagation() // prevent event bubbling
    e.preventDefault()
    const { form } = this.props
    const { paymentItems } = this.state
    form.validateFields((err, values) => {
      if (!err) {
        this.setState({
          paymentItems: [...paymentItems, values],
          visible: false
        })
      }
    })
  }

  render() {
    const { getFieldDecorator } = this.props.form
    const formItemLayout = {
      labelCol: {
        span: 24
      },
      wrapperCol: {
        span: 24
      },
      colon: false
    }
    const { paymentItems } = this.state
    return (
      <Wrap>
        <Button onClick={this.showModal}>create payment item</Button>
        <div>
          {paymentItems.map((item, index) => (
            <div key={index}>
              <span>{index}</span>
              <span>{item.amount}</span>
              <span>{item.reason}</span>
              <span>{item.criteria}</span>
            </div>
          ))}
        </div>
        <Modal
          className="project-detail-nobar"
          maskClosable={false}
          visible={this.state.visible}
          onCancel={this.hideModal}
          footer={null}
          width="70%"
        >
          <Form onSubmit={this.handleSubmit}>
            <FormItem label="Amount" {...formItemLayout}>
              {getFieldDecorator('amount', {
                rules: [{ required: true, message: '' }]
              })(<Input />)}
            </FormItem>
            <FormItem label="Reason" {...formItemLayout}>
              {getFieldDecorator('reason', {
                rules: [{ required: true, message: '' }]
              })(<Input />)}
            </FormItem>
            <FormItem label="Criteria of payment" {...formItemLayout}>
              {getFieldDecorator('criteria', {
                rules: [{ required: true, message: '' }]
              })(<Input />)}
            </FormItem>

            <Button className="cr-btn cr-btn-default" onClick={this.hideModal}>
              {I18N.get('suggestion.cancel')}
            </Button>

            <Button
              // onClick={this.handleSubmit}
              className="cr-btn cr-btn-primary"
              htmlType="submit"
            >
              {I18N.get('suggestion.submit')}
            </Button>
          </Form>
        </Modal>
      </Wrap>
    )
  }
}

export default Form.create()(PaymentSchedule)

const Wrap = styled.div``
