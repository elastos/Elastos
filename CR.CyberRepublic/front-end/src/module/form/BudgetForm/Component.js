import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Form, Input, Button } from 'antd'
import BaseComponent from '@/model/BaseComponent'
import CodeMirrorEditor from '@/module/common/CodeMirrorEditor'
import I18N from '@/I18N'

const FormItem = Form.Item

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

  validateAmount = (rule, value, cb) => {
    const reg = /^(0|[1-9][0-9]*)(\.[0-9]*)?$/
    return (!isNaN(value) && reg.test(value)) || value === '' ? cb() : cb(true)
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
            rules: [
              {
                required: true,
                message: I18N.get('suggestion.form.error.required')
              },
              {
                message: I18N.get('suggestion.form.error.isNaN'),
                validator: this.validateAmount
              }
            ],
            initialValue: item && item.amount ? item.amount : ''
          })(<Input />)}
        </FormItem>
        <StyledFormItem>
          <FormItem
            label={I18N.get('suggestion.budget.reasons')}
            {...formItemLayout}
          >
            {getFieldDecorator('reasons', {
              rules: [
                {
                  required: true,
                  message: I18N.get('suggestion.form.error.required')
                }
              ],
              initialValue: item && item.reasons ? item.reasons : ''
            })(
              <CodeMirrorEditor
                content={item && item.reasons ? item.reasons : ''}
                activeKey="reasons"
                name="reasons"
              />
            )}
          </FormItem>
        </StyledFormItem>
        <StyledFormItem>
          <FormItem
            label={I18N.get('suggestion.budget.criteria')}
            {...formItemLayout}
          >
            {getFieldDecorator('criteria', {
              rules: [{ required: true, message: '' }],
              initialValue: item && item.criteria ? item.criteria : ''
            })(
              <CodeMirrorEditor
                content={item && item.criteria ? item.criteria : ''}
                activeKey="criteria"
                name="criteria"
              />
            )}
          </FormItem>
        </StyledFormItem>
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
            {item
              ? I18N.get('suggestion.form.button.update')
              : I18N.get('suggestion.form.button.create')}
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

const StyledFormItem = styled.div`
  .ant-col-24.ant-form-item-label {
    padding: 0;
    margin-bottom: -12px;
  }
`
