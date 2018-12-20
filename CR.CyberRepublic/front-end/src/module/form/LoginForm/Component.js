import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {Form, Icon, Input, Button, Checkbox} from 'antd'
import ReCAPTCHA from 'react-google-recaptcha';
import {RECAPTCHA_KEY} from '@/config/constant';
import I18N from '@/I18N'
import _ from 'lodash'

import './style.scss'

const FormItem = Form.Item

class C extends BaseComponent {

    constructor(props) {
        super(props)

        this.state = {
            persist: true
        }
    }

    handleSubmit(e) {
        e.preventDefault()
        this.props.form.validateFields((err, values) => {
            if (!err) {
                this.props.login(values.username, values.password, this.state.persist).then(() => {
                    if (_.isFunction(this.props.onHideModal)) {
                        this.props.onHideModal()
                    }
                })
            }
        })
    }

    getInputProps() {
        const {getFieldDecorator} = this.props.form
        const userName_fn = getFieldDecorator('username', {
            rules: [{required: true, message: I18N.get('login.label_username')}],
            initialValue: ''
        })
        const userName_el = (
            <Input size="large"
                placeholder={I18N.get('login.username')}/>
        )

        const pwd_fn = getFieldDecorator('password', {
            rules: [{required: true, message: I18N.get('login.label_password')}]
        })
        const pwd_el = (
            <Input size="large"
                type="password" placeholder={I18N.get('login.password')}/>
        )

        const persist_fn = getFieldDecorator('persist')
        const persist_el = (
            <Checkbox className="checkbox pull-left" onClick={this.togglePersist.bind(this)} checked={this.state.persist}>{I18N.get('login.logged')}</Checkbox>
        )

        return {
            userName: userName_fn(userName_el),
            pwd: pwd_fn(pwd_el),
            persist: persist_fn(persist_el)
        }
    }

    togglePersist() {
        this.setState({persist: !this.state.persist})
    }

    ord_render() {
        const p = this.getInputProps()
        return (
            <Form onSubmit={this.handleSubmit.bind(this)} className="c_loginForm">
                <FormItem>
                    {p.userName}
                </FormItem>
                <FormItem>
                    {p.pwd}
                </FormItem>
                <FormItem>
                    {p.persist}
                    <a className="login-form-forgot pull-right" onClick={() => this.props.history.push('/forgot-password')}>{I18N.get('login.forget')}</a>
                </FormItem>
                <FormItem>
                    <Button loading={this.props.loading} type="ebp" htmlType="submit" className="d_btn d_btn_join">
                        {I18N.get('login.submit')}
                    </Button>
                </FormItem>
            </Form>
        )
    }
}

export default Form.create()(C)
