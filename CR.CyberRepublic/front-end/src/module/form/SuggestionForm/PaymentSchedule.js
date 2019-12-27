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
    this.setState({ visible: true, index: -1 })
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

  handleEdit = index => {
    this.setState({ index, visible: true })
  }

  handleSubmit = values => {
    const { paymentItems, index } = this.state
    if (index >= 0) {
      const rs = paymentItems.map((item, key) => {
        if (index === key) {
          return values
        }
        return item
      })
      this.setState({ paymentItems: rs, visible: false }, () => {
        this.changeValue(this.state.paymentItems)
      })
      return
    }
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
    const { paymentItems, index } = this.state
    return (
      <Wrapper>
        <Header>
          <Label>{I18N.get('suggestion.budget.schedule')}</Label>
          <Button onClick={this.showModal}>
            {I18N.get('suggestion.budget.create')}
          </Button>
        </Header>
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
          width={770}
        >
          {this.state.visible === true ? (
            <BudgetForm
              item={index >= 0 ? paymentItems[index] : null}
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
  callback: PropTypes.func,
  initialValue: PropTypes.array
}

export default PaymentSchedule

const Wrapper = styled.div``
const Header = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  .ant-btn {
    border: 1px solid #000000;
    color: #000000;
    &:hover {
      border: 1px solid #008d85;
      color: #008d85;
    }
  }
`
const Label = styled.div`
  font-size: 17px;
  line-height: 24px;
  color: #000000;
`
