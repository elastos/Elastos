import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Form, Input, Button, Row, message, Modal,
} from 'antd'
import I18N from '@/I18N'
import _ from 'lodash'
import { CONTENT_TYPE } from '@/constant'
import { convertToRaw } from 'draft-js'
import DraftEditor from '@/module/common/DraftEditor'

// if using webpack
import 'medium-draft/lib/index.css'

import { Container, Title, StyledFormItem, Note, NoteHighlight } from './style'

const FormItem = Form.Item

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

const renderRichEditor = (data, key, getFieldDecorator, max) => {
  const content = _.get(data, key, '')
  const rules = [
    {
      required: true,
      transform,
      message: I18N.get('proposal.form.error.required')
    },
  ]
  if (max) {
    rules.push({
      max,
      transform,
      message: I18N.get('proposal.form.error.tooLong')
    })
  }
  const content_fn = getFieldDecorator(key, {
    rules,
    validateTrigger: 'onSubmit',
    initialValue: content,
  })
  const content_el = (
    <DraftEditor content={content} contentType={CONTENT_TYPE.MARKDOWN} />
  )
  return content_fn(content_el)
}

const formatValue = (value) => {
  let result
  try {
    result = _.isString(value) ? value : JSON.stringify(convertToRaw(value.getCurrentContent()))
  } catch (error) {
    result = _.toString(value)
  }
  return result
}

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      loading: false,
    }
    this.user = this.props.user
  }


  ord_loading(f = false) {
    this.setState({ loading: f })
  }

  publishCVote = async (e) => {
    e.preventDefault()
    const { form, create, onCreated, proposal } = this.props

    form.validateFields(async (err, values) => {
      if (err) return
      const { content } = values
      const param = {
        proposalId: proposal._id,
        content: formatValue(content),
      }

      this.ord_loading(true)
      try {
        await create(param)
        this.ord_loading(false)
        await onCreated()
        message.success(I18N.get('from.CVoteForm.message.updated.success'))
      } catch (error) {
        message.error(error.message)
        this.ord_loading(false)
      }
    })
  }

  getInputProps() {
    const { getFieldDecorator } = this.props.form
    const content = renderRichEditor(undefined, 'content', getFieldDecorator)

    return {
      content,
    }
  }

  ord_render() {
    const { proposal, currentUserId } = this.props
    if (proposal.proposer._id !== currentUserId) {
      return null
    }
    const formProps = this.getInputProps()

    return (
      <Container>
        <Form onSubmit={this.publishCVote}>
          <Title>
            {this.props.header || I18N.get('proposal.form.tracking.add')}
          </Title>
          <StyledFormItem>
            <Note>{I18N.get('proposal.form.note.tracking')}</Note>
            <FormItem>{formProps.content}</FormItem>
          </StyledFormItem>
          <Row gutter={8} type="flex" justify="center">
            {this.renderSaveBtn()}
          </Row>
        </Form>
      </Container>
    )
  }

  renderSaveBtn() {
    const btnText = I18N.get('proposal.form.tracking.btn.submit')
    return (
      <FormItem>
        <Button loading={this.state.loading} className="cr-btn cr-btn-primary" htmlType="submit">
          {btnText}
        </Button>
      </FormItem>
    )
  }
}

export default Form.create()(C)
