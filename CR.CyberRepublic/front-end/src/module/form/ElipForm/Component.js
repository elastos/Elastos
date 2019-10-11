import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Input, Button, Popconfirm, message } from 'antd'
import I18N from '@/I18N'
import CodeMirrorEditor from '@/module/common/CodeMirrorEditor'
import ElipNote from '@/module/page/elip/ElipNote'
import { ELIP_STATUS } from '@/constant'
import { Container, Title, Actions, Label, Status } from './style'

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
  saveElip = () => {
    const { history, create, form, data, update } = this.props
    form.validateFields(async (err, values) => {
      if (err) {
        return
      }
      const param = {
        title: values.title,
        description: values.description
      }
      if (!data) {
        const elip = await create(param)
        message.info(I18N.get('elip.msg.submitted'))
        history.push(`/elips/${elip._id}`)
      } else {
        param._id = data._id
        await update(param)
        message.info(I18N.get('elip.msg.updated'))
        history.push(`/elips/${data._id}`)
      }
    })
  }

  handleSubmit = e => {
    e.preventDefault()
    this.saveElip()
  }

  ord_render() {
    const { form, data } = this.props
    const { getFieldDecorator } = form
    return (
      <Container>
        <Title className="komu-a cr-title-with-icon ">
          {data
            ? `${I18N.get('elip.button.edit')} ELIP #${data.vid}`
            : I18N.get('elip.button.add')}
        </Title>
        {data && data.status === ELIP_STATUS.REJECTED && (
          <div>
            <Label>Status</Label>
            <Status>{data.status}</Status>
          </div>
        )}
        <Form>
          <FormItem
            label={`${I18N.get('elip.fields.title')}`}
            {...formItemLayout}
          >
            {getFieldDecorator('title', {
              rules: [
                {
                  required: true,
                  message: I18N.get('elip.form.error.required')
                }
              ],
              initialValue: data && data.title ? data.title : ''
            })(<Input />)}
          </FormItem>
          <ElipNote />
          <FormItem
            label={`${I18N.get('elip.fields.description')}`}
            {...formItemLayout}
          >
            {getFieldDecorator('description', {
              rules: [
                {
                  required: true,
                  message: I18N.get('elip.form.error.required')
                }
              ]
            })(
              <CodeMirrorEditor
                content={data && data.description ? data.description : ''}
              />
            )}
          </FormItem>
          <Actions>
            <Button
              onClick={() => this.props.history.push('/elips')}
              className="cr-btn cr-btn-default"
              style={{ marginRight: 10 }}
            >
              {I18N.get('elip.button.cancel')}
            </Button>

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
          </Actions>
        </Form>
      </Container>
    )
  }
}

export default Form.create()(C)
