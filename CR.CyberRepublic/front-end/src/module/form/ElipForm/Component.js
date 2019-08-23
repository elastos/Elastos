import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Input, Button, Popconfirm } from 'antd'
import I18N from '@/I18N'
import DraftEditor from '@/module/common/DraftEditor'
import ElipNote from '@/module/page/elip/ElipNote'
import { Container, Title, Actions } from './style'

const FormItem = Form.Item
const formItemLayout = {
  labelCol: {
    span: 3
  },
  wrapperCol: {
    span: 17
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
        <Form>
          <FormItem
            label={`${I18N.get('elip.fields.title')}*`}
            {...formItemLayout}
          >
            {getFieldDecorator('title', {
              rules: [
                {
                  required: true,
                  message: I18N.get('elip.form.error.required')
                }
              ]
            })(<Input />)}
          </FormItem>
          <ElipNote />
          <FormItem
            label={`${I18N.get('elip.fields.description')}*`}
            {...formItemLayout}
          >
            {getFieldDecorator('description', {
              rules: [
                {
                  required: true,
                  message: I18N.get('elip.form.error.required')
                }
              ]
            })(<DraftEditor contentType="MARKDOWN" />)}
          </FormItem>
          <Actions>
            <FormItem>
              <Button
                onClick={() => this.props.history.push('/elips')}
                className="cr-btn cr-btn-default"
                style={{ marginRight: 10 }}
              >
                {I18N.get('elip.button.cancel')}
              </Button>
            </FormItem>
            <FormItem>
              <Popconfirm
                title={I18N.get('elip.modal.submit')}
                onConfirm={e => this.handleSubmit(e)}
                okText={I18N.get('.yes')}
                cancelText={I18N.get('.no')}
              >
                <Button
                  loading={this.state.loading}
                  className="cr-btn cr-btn-primary"
                >
                  {I18N.get('elip.button.submit')}
                </Button>
              </Popconfirm>
            </FormItem>
          </Actions>
        </Form>
      </Container>
    )
  }
}

export default Form.create()(C)
