import React, { Component } from 'react'
import PropTypes from 'prop-types'
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

  changeValue(value) {
    const { onChange, callback } = this.props
    onChange(value)
    callback()
  }

  handleSubmit = e => {
    e.stopPropagation() // prevent event bubbling
    e.preventDefault()
    const { form } = this.props
    const { paymentItems } = this.state
    form.validateFields((err, values) => {
      if (!err) {
        this.setState(
          {
            paymentItems: [...paymentItems, values],
            visible: false
          },
          () => {
            this.changeValue(this.state.paymentItems)
          }
        )
      }
    })
  }

  renderPaymentTable = paymentItems => {
    return (
      <StyledTable>
        <StyledHead>
          <StyledRow>
            <th>{I18N.get('suggestion.budget.payment')}#</th>
            <th>{I18N.get('suggestion.budget.amount')}(ELA)</th>
            <th>{I18N.get('suggestion.budget.reasons')}</th>
            <th>{I18N.get('suggestion.budget.criteria')}</th>
            <th>{I18N.get('suggestion.budget.action')}</th>
          </StyledRow>
        </StyledHead>
        <tbody>
          {paymentItems.map((item, index) => (
            <StyledRow key={index}>
              <td>{index}</td>
              <td>{item.amount}</td>
              <td>{item.reasons}</td>
              <td>{item.criteria}</td>
              <td>Action</td>
            </StyledRow>
          ))}
        </tbody>
      </StyledTable>
    )
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
      <Wrapper>
        <Button onClick={this.showModal}>{I18N.get('suggestion.budget.create')}</Button>
        {paymentItems.length ? this.renderPaymentTable(paymentItems) : null}
        <Modal
          className="project-detail-nobar"
          maskClosable={false}
          visible={this.state.visible}
          onCancel={this.hideModal}
          footer={null}
          width="70%"
        >
          <Form onSubmit={this.handleSubmit}>
            <FormItem
              label={`${I18N.get('suggestion.budget.amount')}(ELA)`}
              {...formItemLayout}
            >
              {getFieldDecorator('amount', {
                rules: [{ required: true, message: '' }]
              })(<Input />)}
            </FormItem>
            <FormItem
              label={I18N.get('suggestion.budget.reasons')}
              {...formItemLayout}
            >
              {getFieldDecorator('reasons', {
                rules: [{ required: true, message: '' }]
              })(<Input />)}
            </FormItem>
            <FormItem
              label={I18N.get('suggestion.budget.criteria')}
              {...formItemLayout}
            >
              {getFieldDecorator('criteria', {
                rules: [{ required: true, message: '' }]
              })(<Input />)}
            </FormItem>

            <Actions>
              <Button className="cr-btn cr-btn-default" onClick={this.hideModal}>
                {I18N.get('suggestion.cancel')}
              </Button>
              <Button className="cr-btn cr-btn-primary" htmlType="submit">
                {I18N.get('suggestion.submit')}
              </Button>
            </Actions>
          </Form>
        </Modal>
      </Wrapper>
    )
  }
}

PaymentSchedule.propTypes = {
  onChange: PropTypes.func,
  callback: PropTypes.func
}

export default Form.create()(PaymentSchedule)

const Wrapper = styled.div``

const StyledTable = styled.table`
  margin-top: 16px;
  width: 100%;
  font-size: 13px;
`
const StyledHead = styled.thead`
  > tr {
    background: #0f2631;
  }
  th {
    line-height: 18px;
    padding: 16px;
    color: #fff;
    &:first-child {
      width: 80px;
    }
    &:nth-child(2) {
      width: 120px;
    }
    &:last-child {
      width: 120px;
    }
  }
`
const StyledRow = styled.tr`
  width: 100%;
  background: #f2f6fb;
  > td {
    line-height: 18px;
    padding: 16px;
    color: #000;
  }
`
const Actions = styled.div`
  display: flex;
  justify-content: center;
  > button {
    margin: 0 8px;
  }
`
