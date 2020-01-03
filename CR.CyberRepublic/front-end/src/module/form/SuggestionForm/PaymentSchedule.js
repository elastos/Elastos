import React, { Component } from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Button, Modal, Input } from 'antd'
import I18N from '@/I18N'
import BudgetForm from '@/module/form/BudgetForm/Container'
import PaymentList from './PaymentList'

class PaymentSchedule extends Component {
  constructor(props) {
    super(props)
    this.state = {
      visible: false,
      total: props.budgetAmount || '',
      address: props.elaAddress || '',
      paymentItems: props.initialValue || []
    }
  }

  changeValue(value) {
    const { onChange, callback } = this.props
    onChange(value)
    callback('budget')
  }

  passDataToParent() {
    const { total, address, paymentItems } = this.state
    this.changeValue({
      budgetAmount: Number(total),
      elaAddress: address,
      budget: paymentItems
    })
  }

  handleChange = (e, field) => {
    this.setState({ [field]: e.target.value }, () => {
      this.passDataToParent()
    })
  }

  hideModal = () => {
    this.setState({ visible: false })
  }

  showModal = () => {
    this.setState({ visible: true, index: -1 })
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
        this.passDataToParent()
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
        this.passDataToParent()
      })
      return
    }
    this.setState(
      {
        paymentItems: [...paymentItems, values],
        visible: false
      },
      () => {
        this.passDataToParent()
      }
    )
  }

  render() {
    const { paymentItems, index, total, address } = this.state
    return (
      <Wrapper>
        <Section>
          <Label>{I18N.get('suggestion.budget.total')}</Label>
          <StyledInput
            value={total}
            onChange={e => this.handleChange(e, 'total')}
          />
        </Section>
        <Section>
          <Label>{I18N.get('suggestion.budget.address')}</Label>
          <StyledInput
            value={address}
            onChange={e => this.handleChange(e, 'address')}
          />
        </Section>
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
              types={paymentItems.map(item => item.type)}
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
  margin-top: 24px;
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
const Section = styled.div`
  margin-top: 24px;
`
const StyledInput = styled.input`
  outline: 0;
  background: #ffffff;
  border: 1px solid #d9d9d9;
  margin-top: 16px;
  padding: 6px 11px;
  width: 50%;
  height: 40px;
  transition: all 0.3s;
  &:hover {
    border-color: #66bda3;
  }
  &:focus {
    border-color: #66bda3;
    box-shadow: 0 0 0 2px rgba(67, 175, 146, 0.2);
  }
`
