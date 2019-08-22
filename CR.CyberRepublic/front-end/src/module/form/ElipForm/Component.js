import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Input, Button } from 'antd'
import I18N from '@/I18N'
import { Container, Title } from './style'

const FormItem = Form.Item
const formItemLayout = {
  labelCol: {
    span: 2
  },
  wrapperCol: {
    span: 18
  },
  colon: false
}

class C extends BaseComponent {
  constructor(p) {
    super(p)
  }

  handleSubmit = e => {
    e.preventDefault()
    console.log('handle submit')
  }

  ord_render() {
    const { form } = this.props
    const { getFieldDecorator } = form
    return (
      <Container>
        <Title className="komu-a cr-title-with-icon ">
          {I18N.get('elip.button.add')}
        </Title>
        <Form onSubmit={this.handleSubmit}>
          <FormItem
            label={`${I18N.get('elip.fields.title')}*`}
            {...formItemLayout}
          >
            {getFieldDecorator('title', {
              rules: [{ required: true, message: 'Please input your username!' }],
            })(
              <Input placeholder="title" />
            )}
          </FormItem>
        </Form>
      </Container>
    )
  }
}

export default Form.create()(C)
