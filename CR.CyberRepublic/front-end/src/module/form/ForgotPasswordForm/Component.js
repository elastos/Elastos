import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {Form, Icon, Input, Button} from 'antd'
import ReCAPTCHA from 'react-google-recaptcha'
import {RECAPTCHA_KEY} from '@/config/constant'
import I18N from '@/I18N'

import './style.scss'

const FormItem = Form.Item

/**
 * We may require a username retrieval process and we should hide it in the future
 */
class C extends BaseComponent {

  handleSubmit(e) {
    e.preventDefault()
    this.props.form.validateFields(async (err, values) => {
      if (!err) {
        // console.log('Received values of form: ', values)
        await this.props.forgotPassword(values.email)

        this.props.form.resetFields()
      }
    })
  }

  getInputProps() {
    const {getFieldDecorator} = this.props.form
    const email_fn = getFieldDecorator('email', {
      rules: [{required: true, message: I18N.get('forgot.form.label_email')}],
      initialValue: ''
    })
    const email_el = (
      <Input size="large"
        prefix={<Icon type="user" style={{color: 'rgba(0,0,0,.25)'}}/>}
        placeholder={I18N.get('forgot.form.email')}/>
    )

    return {
      email: email_fn(email_el)
    }
  }

  ord_render() {
    const {getFieldDecorator} = this.props.form
    const p = this.getInputProps()
    return (
      <Form onSubmit={this.handleSubmit.bind(this)} className="c_loginForm">
        <FormItem>
          {p.email}
        </FormItem>
        <FormItem style={{margin: '0 20%'}}>
          <Button loading={this.props.loading} type="ebp" htmlType="submit" className="d_btn resetPassword">
            {I18N.get('forgot.form.submit')}
          </Button>
        </FormItem>
      </Form>
    )
  }
}

export default Form.create()(C)
