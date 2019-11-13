import React from 'react'
import PropTypes from 'prop-types'
import BaseComponent from '@/model/BaseComponent'
import styled from 'styled-components'
import { Form, Input, Button } from 'antd'
import I18N from '@/I18N'

const FormItem = Form.Item
const TextArea = Input.TextArea

class TeamInfoForm extends BaseComponent {
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
          label={I18N.get('suggestion.plan.teamMember')}
          {...formItemLayout}
        >
          {getFieldDecorator('member', {
            rules: [
              {
                required: true,
                message: I18N.get('suggestion.form.error.required')
              }
            ],
            initialValue: item && item.member ? item.member : ''
          })(<Input />)}
        </FormItem>
        <FormItem label={I18N.get('suggestion.plan.role')} {...formItemLayout}>
          {getFieldDecorator('role', {
            rules: [
              {
                required: true,
                message: I18N.get('suggestion.form.error.required')
              }
            ],
            initialValue: item && item.role ? item.role : ''
          })(<Input />)}
        </FormItem>
        <FormItem
          label={I18N.get('suggestion.plan.responsibility')}
          {...formItemLayout}
        >
          {getFieldDecorator('responsibility', {
            rules: [
              {
                required: true,
                message: I18N.get('suggestion.form.error.required')
              }
            ],
            initialValue: item && item.responsibility ? item.responsibility : ''
          })(<TextArea rows={5} />)}
        </FormItem>
        <FormItem
          label={I18N.get('suggestion.plan.moreInfo')}
          {...formItemLayout}
        >
          {getFieldDecorator('info', {
            rules: [{ required: true, message: '' }],
            initialValue: item && item.info ? item.info : ''
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

TeamInfoForm.propTypes = {
  onSubmit: PropTypes.func.isRequired,
  onCancel: PropTypes.func.isRequired,
  item: PropTypes.object
}

export default Form.create()(TeamInfoForm)

const Actions = styled.div`
  display: flex;
  justify-content: center;
  > button {
    margin: 0 8px;
  }
`
