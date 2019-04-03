import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Form,
  Input,
  Button,
  message

} from 'antd'

import {upload_file} from '@/util'
import I18N from '@/I18N'
import './style.scss'

import {TASK_CATEGORY, TASK_TYPE, TASK_STATUS} from '@/constant'

const FormItem = Form.Item
const TextArea = Input.TextArea

class C extends BaseComponent {

  handleSubmit (e) {
    e.preventDefault()
    this.props.form.validateFields((err, formData) => {
      if (!err) {
        this.props.sendEmail(this.props.recipient._id, formData).then(() => {
          message.success(I18N.get('from.UserContactForm.message.success'))
        })
        this.props.close && this.props.close()
      }
    })
  }

  getInputProps () {

    const {getFieldDecorator} = this.props.form

    const subject_fn = getFieldDecorator('subject', {
      rules: []
    })
    const subject_el = (
      <Input size="large" placeholder="subject"/>
    )

    const message_fn = getFieldDecorator('message', {
      rules: [{required: true, message: I18N.get('from.UserContactForm.field.required')}]
    })
    const message_el = (
      <TextArea rows={4} name="message" placeholder={I18N.get('from.UserContactForm.placeholder.message')} />
    )

    const name = `${this.props.recipient.profile.firstName} ${this.props.recipient.profile.lastName}`
    const defaultSubject = `Cyber Republic - Message from ${name}`
    return {
      subject: subject_fn(subject_el) || defaultSubject,
      message: message_fn(message_el)
    }
  }

  ord_render () {
    const {getFieldDecorator} = this.props.form
    const p = this.getInputProps()

    // TODO: description CKE Editor
    return (
      <div className="c_taskCreateFormContainer">

        <span className="no-info">
          {I18N.get('from.UserContactForm.text.emailreply')}
        </span>
        <br/>
        <Form onSubmit={this.handleSubmit.bind(this)} className="d_userContactForm">
          <div>
            <FormItem>
              {p.subject}
            </FormItem>
            <FormItem>
              {p.message}
            </FormItem>

            <FormItem wrapperCol={{xs: {span: 24, offset: 0}, sm: {span: 12, offset: 0}}}>
              <Button loading={this.props.loading} type="ebp" htmlType="submit" className="d_btn">
                {I18N.get('from.UserContactForm.button.send')}
              </Button>
            </FormItem>
          </div>
        </Form>
      </div>
    )
  }

}
export default Form.create()(C)
