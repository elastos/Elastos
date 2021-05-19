import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Input, Button, Select, Divider } from 'antd'
// import ReCAPTCHA from 'react-google-recaptcha'
import { RECAPTCHA_KEY, MIN_LENGTH_PASSWORD } from '@/config/constant'
import config from '@/config'
import I18N from '@/I18N'
import _ from 'lodash'

import './style.scss'

const FormItem = Form.Item

class C extends BaseComponent {
  state = {
    requestedCode: null
  }

  handleSubmit(e) {
    e.preventDefault()

    // eslint-disable-next-line no-undef
    analytics.track('REGISTRATION', {
      action: 'registration submit button clicked',
      url: location.href
    })

    this.props.form.validateFields(async (err, values) => {
      if (!err) {
        if (this.state.requestedCode) {
          // eslint-disable-next-line no-undef
          analytics.track('REGISTRATION', {
            action: 'registration successful',
            url: location.href,
            username: this.state.savedValues.username,
            email: this.state.savedValues.email
          })

          this.props
            .register(
              this.state.savedValues.username,
              this.state.savedValues.password,
              this.props.did,
              this.state.savedValues.email
            )
            .then(shouldShowWelcome => {
              if (_.isFunction(this.props.onChangeActiveKey)) {
                if (shouldShowWelcome) {
                  this.props.onChangeActiveKey('post')
                } else {
                  this.props.onChangeActiveKey('login')
                }
              }
            })
        } else {
          // eslint-disable-next-line no-undef
          analytics.track('REGISTRATION', {
            action: 'sent registration code',
            url: location.href,
            username: values.username,
            email: values.email
          })

          // step 1 - check username, if valid send registration code
          const code = this.generateRegCode()
          this.props.sendRegistrationCode(values.email, code)
          this.setState({
            requestedCode: code,
            savedValues: values
          })
          if (_.isFunction(this.props.onHideTabBar)) {
            this.props.onHideTabBar()
          }
        }
      }
    })
  }

  // TODO: move to back-end
  generateRegCode() {
    // Generate a random six digit code
    const min = 100000
    const max = 1000000
    return Math.round(Math.random() * (max - min) + min)
  }

  checkEmail(rule, value, callback, source, options) {
    // eslint-disable-next-line no-useless-escape
    const emailRegex = new RegExp(
      /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/
    )

    if (value && emailRegex.test(value)) {
      this.props.checkEmail(value).then(isExist => {
        if (isExist) {
          callback(I18N.get('register.error.duplicate_email'))
        } else {
          callback()
        }
      })
    } else {
      callback()
    }
  }

  validateRegCode(rule, value, callback) {
    const reqCode = this.state.requestedCode
    const form = this.props.form

    if (reqCode && reqCode.toString() !== value) {
      callback(I18N.get('register.error.code')) // The code you entered does not match
    } else {
      callback()
    }
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
      callback(
        `${I18N.get(
          'register.error.password_length_1'
        )} ${MIN_LENGTH_PASSWORD} ${I18N.get(
          'register.error.password_length_2'
        )}`
      )
    }
    callback()
  }

  getConfirmInputProps() {
    const { getFieldDecorator } = this.props.form

    const regCode_fn = getFieldDecorator('reg_code', {
      rules: [
        { required: true, message: I18N.get('register.form.input_code') },
        { validator: this.validateRegCode.bind(this) }
      ]
    })
    const regCode_el = (
      <Input
        size="large"
        placeholder={I18N.get('register.form.confirmation_code')}
      />
    )

    return {
      regCode: regCode_fn(regCode_el)
    }
  }

  getInputProps() {
    const { getFieldDecorator } = this.props.form

    const firstName_fn = getFieldDecorator('firstName', {
      rules: [],
      initialValue: ''
    })
    const firstName_el = (
      <Input size="large" placeholder={I18N.get('register.form.first_name')} />
    )

    const lastName_fn = getFieldDecorator('lastName', {
      rules: [],
      initialValue: ''
    })
    const lastName_el = (
      <Input size="large" placeholder={I18N.get('register.form.last_name')} />
    )

    const username_fn = getFieldDecorator('username', {
      rules: [
        { required: true, message: I18N.get('register.form.label_username') },
        { min: 6, message: I18N.get('register.error.username') }
      ],
      initialValue: ''
    })
    const username_el = (
      <Input size="large" placeholder={I18N.get('register.form.username')} />
    )

    const email_fn = getFieldDecorator('email', {
      rules: [
        {
          required: true,
          message: I18N.get('register.form.label_email')
        },
        {
          type: 'email',
          message: I18N.get('register.error.email')
        },
        {
          validator: this.checkEmail.bind(this)
        }
      ]
    })
    const email_el = (
      <Input size="large" placeholder={I18N.get('register.form.email')} />
    )

    const pwd_fn = getFieldDecorator('password', {
      rules: [
        {
          required: true,
          message: I18N.get('register.form.label_password')
        },
        {
          validator: this.validateToNextPassword.bind(this)
        }
      ]
    })
    const pwd_el = (
      <Input
        size="large"
        type="password"
        placeholder={I18N.get('register.form.password')}
      />
    )

    const pwdConfirm_fn = getFieldDecorator('passwordConfirm', {
      rules: [
        {
          required: true,
          message: I18N.get('register.form.label_password_confirm')
        },
        {
          validator: this.compareToFirstPassword.bind(this)
        }
      ]
    })
    const pwdConfirm_el = (
      <Input
        size="large"
        type="password"
        placeholder={I18N.get('register.form.password_confirm')}
      />
    )

    const country_fn = getFieldDecorator('country', {
      rules: []
    })
    const country_el = (
      <Select
        size="large"
        showSearch={true}
        filterOption={(input, option) =>
          option.props.children.toLowerCase().indexOf(input.toLowerCase()) >= 0
        }
        placeholder={I18N.get('register.form.option')}
      >
        {_.entries(config.data.mappingCountryCodeToName).map(([key, val]) => {
          return (
            <Select.Option key={key} value={key}>
              {val}
            </Select.Option>
          )
        })}
      </Select>
    )

    // const recaptcha_fn = getFieldDecorator('recaptcha', {
    //   rules: [{ required: true }]
    // })
    // const recaptcha_el = (
    //   <ReCAPTCHA
    //     ref={el => {
    //       this.captcha = el
    //     }}
    //     sitekey={RECAPTCHA_KEY}
    //   />
    // )

    const organizer_fn = getFieldDecorator('beOrganizer', {
      rules: [{ message: I18N.get('register.form.options') }]
    })
    const organizer_el = (
      <Select size="large" placeholder={I18N.get('register.form.organizer')}>
        <Select.Option value="yes">
          {I18N.get('register.form.yes')}
        </Select.Option>
        <Select.Option value="no">{I18N.get('register.form.no')}</Select.Option>
      </Select>
    )

    const developer_fn = getFieldDecorator('isDeveloper', {
      rules: [{ message: I18N.get('register.form.option') }]
    })
    const developer_el = (
      <Select size="large" placeholder={I18N.get('register.form.developer')}>
        <Select.Option value="yes">
          {I18N.get('register.form.yes')}
        </Select.Option>
        <Select.Option value="no">{I18N.get('register.form.no')}</Select.Option>
      </Select>
    )

    const source_fn = getFieldDecorator('source', {
      rules: [{ message: '' }]
    })
    const source_el = (
      <Input size="large" placeholder={I18N.get('register.form.hear')} />
    )

    return {
      firstName: firstName_fn(firstName_el),
      lastName: lastName_fn(lastName_el),
      userName: username_fn(username_el),
      email: email_fn(email_el),
      pwd: pwd_fn(pwd_el),
      pwdConfirm: pwdConfirm_fn(pwdConfirm_el),

      country: country_fn(country_el),

      organizer: organizer_fn(organizer_el),
      developer: developer_fn(developer_el),

      source: source_fn(source_el)

      // recaptcha: recaptcha_fn(recaptcha_el)
    }
  }

  getForm() {
    if (this.state.requestedCode) {
      const p = this.getConfirmInputProps()
      return (
        <div>
          <h3 className="citizen-title komu-a">
            {I18N.get('register.code.title')}
          </h3>
          <Form
            onSubmit={this.handleSubmit.bind(this)}
            className="d_registerForm"
          >
            <FormItem>{p.regCode}</FormItem>
            <FormItem>
              <Button
                loading={this.props.loading}
                type="ebp"
                htmlType="submit"
                className="d_btn d_btn_join"
                onClick={this.handleSubmit.bind(this)}
              >
                {I18N.get('register.submit')}
              </Button>
            </FormItem>
            <Divider className="code-sent-text">
              {I18N.get('register.code')}
              <span className="code-email">{this.state.savedValues.email}</span>
            </Divider>
          </Form>
        </div>
      )
    }
    const p = this.getInputProps()
    return (
      <Form onSubmit={this.handleSubmit.bind(this)} className="d_registerForm">
        <FormItem>{p.email}</FormItem>
        <FormItem>{p.userName}</FormItem>
        <FormItem>{p.pwd}</FormItem>
        <FormItem>{p.pwdConfirm}</FormItem>
        {/* <FormItem>{p.recaptcha}</FormItem> */}
        <FormItem>
          <Button
            loading={this.props.loading}
            type="ebp"
            htmlType="submit"
            className="d_btn d_btn_join"
            onClick={this.handleSubmit.bind(this)}
          >
            {I18N.get('register.submit')}
          </Button>
        </FormItem>
      </Form>
    )
  }

  ord_render() {
    const form = this.getForm()

    return <div className="c_registerContainer">{form}</div>
  }
}

export default Form.create()(C)
