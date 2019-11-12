import React, { Component } from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Button, Modal } from 'antd'
import I18N from '@/I18N'
import BudgetForm from '@/module/form/BudgetForm/Container'
import PaymentList from './PaymentList'

class PaymentSchedule extends Component {
  constructor(props) {
    super(props)
    this.state = {
      visible: false,
      paymentItems: props.initialValue ? props.initialValue : []
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
    callback('budget')
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

  handleEdit = (activeIndex, values) => {
    const { paymentItems } = this.state
    const rs = paymentItems.map((item, index) => {
      if (index === activeIndex) {
        return values
      }
      return item
    })
    this.setState({ paymentItems: rs }, () => {
      this.changeValue(this.state.paymentItems)
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

  render() {
    const { paymentItems } = this.state
    return (
      <Wrapper>
        <Button onClick={this.showModal}>
          {I18N.get('suggestion.budget.create')}
        </Button>
        {paymentItems.length ? (
          <PaymentList
            list={paymentItems}
            onDelete={this.handleDelete}
            onEdit={this.handleEdit}
          />
        ) : null}
        <Modal
          maskClosable={false}
          visible={this.state.visible}
          onCancel={this.hideModal}
          footer={null}
          width="70%"
        >
          {this.state.visible === true ? (
            <BudgetForm
              onSubmit={this.handleSubmit}
              onCancel={this.hideModal}
            />
          ) : null}
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
