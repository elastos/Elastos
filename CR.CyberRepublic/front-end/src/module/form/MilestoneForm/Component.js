import React from 'react'
import PropTypes from 'prop-types'
import { Form, Input, DatePicker } from 'antd'
import styled from 'styled-components'
import BaseComponent from '@/model/BaseComponent'
import I18N from '@/I18N'

const { TextArea } = Input
const FormItem = Form.Item

class MilestoneForm extends BaseComponent {
  handleSubmit = e => {
    e.stopPropagation() // prevent event bubbling
    e.preventDefault()
    const { form, onSubmit, item } = this.props
    form.validateFields((err, values) => {
      if (!err) {
        item ? onSubmit(item.index, values) : onSubmit(values)
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
      <Form style={{ width: 330 }}>
        <FormItem
          label={I18N.get('suggestion.plan.publishDate')}
          {...formItemLayout}
        >
          {getFieldDecorator('date', {
            rules: [
              {
                required: true,
                message: I18N.get('suggestion.form.error.required')
              }
            ],
            initialValue: item && item.date
          })(<DatePicker />)}
        </FormItem>
        <FormItem label={I18N.get('suggestion.plan.goal')} {...formItemLayout}>
          {getFieldDecorator('version', {
            rules: [
              {
                required: true,
                message: I18N.get('suggestion.form.error.required')
              }
            ],
            initialValue: item && item.version
          })(<TextArea rows={8} style={{ resize: 'none' }} />)}
        </FormItem>
        <Actions>
          <Button onClick={this.handleSubmit}>
            {item
              ? I18N.get('suggestion.form.button.update')
              : I18N.get('suggestion.form.button.create')}
          </Button>
        </Actions>
      </Form>
    )
  }
}

MilestoneForm.propTypes = {
  onSubmit: PropTypes.func.isRequired,
  item: PropTypes.object
}

export default Form.create()(MilestoneForm)

const Actions = styled.div`
  display: flex;
  justify-content: center;
`
const Button = styled.div`
  margin: -8px 8px 12px;
  background-color: #008d85;
  width: 90px;
  height: 32px;
  font-size: 13px;
  line-height: 32px;
  text-align: center;
  color: #ffffff;
  cursor: pointer;
`
