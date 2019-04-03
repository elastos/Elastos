import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {Form, Icon, Input, Button} from 'antd'
import {MIN_LENGTH_PASSWORD} from '@/config/constant'
import I18N from '@/I18N'

import './style.scss'

const FormItem = Form.Item

class C extends BaseComponent {

  constructor(props) {
    super(props)

    this.resetToken = ''

    if (this.props.location && this.props.location.search) {
      const tokenQry = this.props.location.search.match(/[\\?&]token=([\w]+)/)
      if (tokenQry && tokenQry.length > 1) {
        this.resetToken = tokenQry[1]
      }
    }
  }

  handleSubmit(e) {
    e.preventDefault()
    this.props.form.validateFields((err, values) => {
      if (!err) {
        // console.log('Received values of form: ', values)
        this.props.resetPassword(this.resetToken, values.password)

      }
    })
  }

  compareToFirstPassword(rule, value, callback) {
    const form = this.props.form
    if (value && value !== form.getFieldValue('password')) {
      callback(I18N.get('register.error.passwords')) // Two passwords you entered do not match'
    } else {
      callback()
    }
  }

  validateToNextPassword(rule, value, callback) {
    const form = this.props.form
    if (value && this.state.confirmDirty) {
      form.validateFields(['confirmPassword'], { force: true })
    }
    if (value && value.length < MIN_LENGTH_PASSWORD) {
      callback(`${I18N.get('register.error.password_length_1')} ${MIN_LENGTH_PASSWORD} ${I18N.get('register.error.password_length_2')}`)
    }
    callback()
  }

  getInputProps() {
    const {getFieldDecorator} = this.props.form
    const pwd_fn = getFieldDecorator('password', {
      rules: [{
        required: true, message: I18N.get('register.form.label_password')
      }, {
        validator: this.validateToNextPassword.bind(this)
      }]
    })
    const pwd_el = (
      <Input size="large"
        prefix={<Icon type="lock" style={{color: 'rgba(0,0,0,.25)'}}/>}
        type="password" placeholder={I18N.get('register.form.password')}/>
    )

    const pwdConfirm_fn = getFieldDecorator('passwordConfirm', {
      rules: [{
        required: true, message: I18N.get('register.form.label_password_confirm')
      }, {
        validator: this.compareToFirstPassword.bind(this)
      }]
    })
    const pwdConfirm_el = (
      <Input size="large"
        prefix={<Icon type="lock" style={{color: 'rgba(0,0,0,.25)'}}/>}
        type="password" placeholder={I18N.get('register.form.password_confirm')}/>
    )

    return {
      pwd: pwd_fn(pwd_el),
      pwdConfirm: pwdConfirm_fn(pwdConfirm_el)
    }
  }

  ord_render() {

    const {getFieldDecorator} = this.props.form
    const p = this.getInputProps()
    return (
      <Form onSubmit={this.handleSubmit.bind(this)} className="c_loginForm">
        <FormItem>
          {p.pwd}
        </FormItem>
        <FormItem>
          {p.pwdConfirm}
        </FormItem>
        <FormItem>
          <Button loading={this.props.loading} type="ebp" htmlType="submit" className="d_btn">
            {I18N.get('login.reset')}
          </Button>
        </FormItem>
      </Form>
    )
  }
}

export default Form.create()(C)
