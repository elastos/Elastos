import React, { Component } from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Button, Modal } from 'antd'
import I18N from '@/I18N'
import BudgetForm from '@/module/form/BudgetForm/Container'

class PaymentSchedule extends Component {
  constructor(props) {
    super(props)
    this.state = {
      visible: false,
      paymentItems: [],
      item: {}
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

  handleDelete = index => {
    const { paymentItems } = this.state
    this.setState(
      {
        paymentItems: [
          ...paymentItems.slice(0, index),
          ...paymentItems.slice(index + 1)
        ]
      },
      () => {
        this.changeValue(this.state.paymentItems)
      }
    )
  }

  handleEdit = index => {
    const { paymentItems } = this.state
    this.setState({
      item: { ...this.state.item, ...paymentItems[index] },
      visible: true
    })
  }

  handleSubmit = values => {
    const { paymentItems } = this.state
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
              <td>{index + 1}</td>
              <td>{item.amount}</td>
              <td>{item.reasons}</td>
              <td>{item.criteria}</td>
              <td>
                <Button onClick={this.handleDelete.bind(this, index)}>
                  delete
                </Button>
                <Button onClick={this.handleEdit.bind(this, index)}>
                  edit
                </Button>
              </td>
            </StyledRow>
          ))}
        </tbody>
      </StyledTable>
    )
  }

  render() {
    const { paymentItems, item } = this.state
    return (
      <Wrapper>
        <Button onClick={this.showModal}>
          {I18N.get('suggestion.budget.create')}
        </Button>
        {paymentItems.length ? this.renderPaymentTable(paymentItems) : null}
        <Modal
          className="project-detail-nobar"
          maskClosable={false}
          visible={this.state.visible}
          onCancel={this.hideModal}
          footer={null}
          width="70%"
        >
          <BudgetForm onSubmit={this.handleSubmit} onCancel={this.hideModal} />
        </Modal>
      </Wrapper>
    )
  }
}

PaymentSchedule.propTypes = {
  onChange: PropTypes.func,
  callback: PropTypes.func
}

export default PaymentSchedule

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
