import React from 'react'
import PropTypes from 'prop-types'
import BaseComponent from '@/model/BaseComponent'
import styled from 'styled-components'
import { Form, Input, Button } from 'antd'
import I18N from '@/I18N'

const FormItem = Form.Item
const TextArea = Input.TextArea

class BudgetForm extends BaseComponent {
  handleSubmit = e => {
    e.stopPropagation() // prevent event bubbling
    e.preventDefault()
    const { form, onSubmit } = this.props
    form.validateFields((err, values) => {
      if (!err) {
        onSubmit(values)
      }
    })
  }

  ord_render() {
    const { getFieldDecorator } = this.props.form
    const { item } = this.props
    const formItemLayout = {
      labelCol: {
        span: 24
      },
      wrapperCol: {
        span: 24
      },
      colon: false
    }
    return (
      <Form onSubmit={this.handleSubmit}>
        <FormItem
          label={`${I18N.get('suggestion.budget.amount')}(ELA)`}
          {...formItemLayout}
        >
          {getFieldDecorator('amount', {
            rules: [{ required: true, message: '' }],
            initialValue: item && item.amount ? item.amount : 0
          })(<Input />)}
        </FormItem>
        <FormItem
          label={I18N.get('suggestion.budget.reasons')}
          {...formItemLayout}
        >
          {getFieldDecorator('reasons', {
            rules: [{ required: true, message: '' }],
            initialValue: item && item.reasons ? item.reasons : ''
          })(<TextArea rows={5} />)}
        </FormItem>
        <FormItem
          label={I18N.get('suggestion.budget.criteria')}
          {...formItemLayout}
        >
          {getFieldDecorator('criteria', {
            rules: [{ required: true, message: '' }],
            initialValue: item && item.criteria ? item.criteria : ''
          })(<TextArea rows={5} />)}
        </FormItem>

        <Actions>
          <Button
            className="cr-btn cr-btn-default"
            onClick={() => {
              this.props.onCancel()
            }}
          >
            {I18N.get('suggestion.cancel')}
          </Button>
          <Button className="cr-btn cr-btn-primary" htmlType="submit">
            {I18N.get('suggestion.submit')}
          </Button>
        </Actions>
      </Form>
    )
  }
}

BudgetForm.propTypes = {
  onSubmit: PropTypes.func.isRequired,
  onCancel: PropTypes.func.isRequired,
  item: PropTypes.object
}

export default Form.create()(BudgetForm)

const Actions = styled.div`
  display: flex;
  justify-content: center;
  > button {
    margin: 0 8px;
  }
`
