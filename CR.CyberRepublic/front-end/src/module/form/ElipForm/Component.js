import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Input, Button, Popconfirm, message } from 'antd'
import { convertToRaw } from 'draft-js'
import I18N from '@/I18N'
import DraftEditor from '@/module/common/DraftEditor'
import ElipNote from '@/module/page/elip/ElipNote'
import { CONTENT_TYPE, ELIP_STATUS, ELIP_DESC_MAX_WORDS } from '@/constant'
import { Container, Title, Actions, Label, Status } from './style'

const FormItem = Form.Item
const WORD_LIMIT = ELIP_DESC_MAX_WORDS
const formItemLayout = {
  labelCol: {
    span: 3
  },
  wrapperCol: {
    span: 17
  },
  colon: false
}
const transform = value => {
  // string or object
  let result = value
  if (_.isObject(value)) {
    try {
      result = value.getCurrentContent().getPlainText()
    } catch (error) {
      result = value
    }
  }
  return result
}
const formatValue = value => {
  let result
  try {
    result = _.isString(value)
      ? value
      : JSON.stringify(convertToRaw(value.getCurrentContent()))
  } catch (error) {
    result = _.toString(value)
  }
  return result
}

class C extends BaseComponent {
  constructor(p) {
    super(p)
  }

  handleSubmit = e => {
    e.preventDefault()
    const { history, create, form, data, update } = this.props
    form.validateFields(async (err, values) => {
      if (err) {
        return
      }
      const param = {
        title: values.title,
        description: formatValue(values.description)
      }
      if (!data) {
        const elip = await create(param)
        message.info(I18N.get('elip.msg.submitted'))
        history.push(`/elips/${elip._id}`)
      } else {
        param._id = data._id
        param.status = ELIP_STATUS.WAIT_FOR_REVIEW
        await update(param)
        message.info(I18N.get('elip.msg.updated'))
        history.push(`/elips/${data._id}`)
      }
    })
  }

  validateDesc = (rule, value, cb) => {
    const { language } = this.props
    let count = value.length
    if (language === 'en') {
      count = value.split(' ').length
    }
    return count > WORD_LIMIT ? cb(true) : cb()
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
                  transform,
                  message: I18N.get('elip.form.error.required')
                },
                {
                  transform,
                  message: I18N.get(`elip.form.error.limit${WORD_LIMIT}`),
                  validator: this.validateDesc
                }
              ],
              initialValue: data && data.description ? data.description : ''
            })(
              <DraftEditor
                contentType={CONTENT_TYPE.MARKDOWN}
                autoFocus={false}
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
