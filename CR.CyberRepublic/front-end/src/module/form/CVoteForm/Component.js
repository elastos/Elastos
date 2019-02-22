import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Form, Input, Button, Select, Row, Col, message, Modal,
} from 'antd'
import I18N from '@/I18N'
import _ from 'lodash'
import { CVOTE_STATUS, CVOTE_TYPE } from '@/constant'

import './style.scss'

const FormItem = Form.Item
const { TextArea } = Input

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      persist: true,
      loading: false,
    }

    this.user = this.props.user
  }

  ord_loading(f = false) {
    this.setState({ loading: f })
  }

  handleSubmit = async (e, fields = {}) => {
    e.preventDefault()
    const fullName = `${this.user.profile.firstName} ${this.user.profile.lastName}`
    const { edit, form, updateCVote, createCVote } = this.props

    form.validateFields(async (err, values) => {
      if (!err) {
        const param = {
          title: values.title,
          type: values.type,
          notes: values.notes,
          motionId: values.motionId,
          isConflict: values.isConflict,
          content: values.content,
          published: true,
          ...fields,
        }
        if (!edit) param.proposedBy = fullName

        this.ord_loading(true)
        if (edit) {
          try {
            param._id = edit
            await updateCVote(param)
            message.success(I18N.get('from.CVoteForm.message.updated.success'))
            this.ord_loading(false)
            this.gotoList()
          } catch (error) {
            message.error(error.message)
            this.ord_loading(false)
          }
        } else {
          try {
            await createCVote(param)
            message.success(I18N.get('from.CVoteForm.message.create.success'))
            this.ord_loading(false)
            this.gotoList()
          } catch (error) {
            message.error(error.message)
            this.ord_loading(false)
          }
        }
      }
    })
  }

  getInputProps(data) {
    const { edit } = this.props
    const s = this.props.static
    const { getFieldDecorator } = this.props.form

    const title_fn = getFieldDecorator('title', {
      rules: [{ required: true }],
      initialValue: edit ? data.title : '',
    })
    const title_el = (
      <Input size="large" type="text" />
    )

    const type_fn = getFieldDecorator('type', {
      rules: [{ required: true }],
      readOnly: true,
      initialValue: edit ? parseInt(data.type, 10) : 1,
    })
    const type_el = (
      <Select size="large">
        {/* <Select.Option key={-1} value={-1}>please select type</Select.Option> */}
        {
          _.map(s.select_type, (item, i) => (
            <Select.Option key={i} value={item.code}>{item.name}</Select.Option>
          ))
        }
      </Select>
    )

    const content_fn = getFieldDecorator('content', {
      rules: [{ required: true }],
      initialValue: edit ? data.content : '',
    })
    const content_el = (
      <TextArea rows={6} />
    )

    const isConflict_fn = getFieldDecorator('isConflict', {
      initialValue: edit ? data.isConflict : 'NO',
    })
    const isConflict_el = (
      <Select size="large">
        <Select.Option value="NO">{I18N.get('from.CVoteForm.yes')}</Select.Option>
        <Select.Option value="YES">{I18N.get('from.CVoteForm.no')}</Select.Option>
      </Select>
    )

    const notes_fn = getFieldDecorator('notes', {
      initialValue: edit ? data.notes : '',
    })
    const notes_el = (
      <TextArea rows={4} />
    )

    return {
      title: title_fn(title_el),
      type: type_fn(type_el),
      content: content_fn(content_el),
      isConflict: isConflict_fn(isConflict_el),
      notes: notes_fn(notes_el),
    }
  }

  togglePersist() {
    const { persist } = this.state
    this.setState({ persist: !persist })
  }

  ord_render() {
    const { edit, data, canManage, isSecretary } = this.props
    let p = null
    if (!canManage || (edit && !data)) {
      return null
    }
    if (edit) {
      p = this.getInputProps(data)
    } else {
      p = this.getInputProps()
    }
    const formItemLayout = {
      labelCol: {
        xs: { span: 24 },
        sm: { span: 6 },
      },
      wrapperCol: {
        xs: { span: 24 },
        sm: { span: 12 },
      },
    }

    return (
      <Form onSubmit={this.handleSubmit} className="c_CVoteForm">
        <h2>
          {I18N.get('from.CVoteForm.proposal.title')}
        </h2>

        <h5>
          {I18N.get('from.CVoteForm.proposal.content')}
        </h5>

        <FormItem label={I18N.get('from.CVoteForm.label.title')} {...formItemLayout} style={{ marginTop: '24px' }}>{ p.title }</FormItem>

        <FormItem label={I18N.get('from.CVoteForm.label.type')} {...formItemLayout}>{p.type}</FormItem>

        <FormItem label={I18N.get('from.CVoteForm.label.content')} {...formItemLayout}>{p.content}</FormItem>

        <FormItem style={{ marginBottom: '12px' }} label={I18N.get('from.CVoteForm.label.conflict')} help={I18N.get('from.CVoteForm.label.conflict.help')} {...formItemLayout}>{p.isConflict}</FormItem>
        {isSecretary && <FormItem label={I18N.get('from.CVoteForm.label.note')} {...formItemLayout}>{p.notes}</FormItem>}
        <Row gutter={8}>
          {this.renderCancelBtn()}
          {this.renderSaveDraftBtn()}
          {this.renderSaveBtn()}
        </Row>
      </Form>
    )
  }

  gotoList = () => {
    this.props.history.push('/proposals')
  }

  saveDraft = (e) => {
    this.handleSubmit(e, { published: false })
  }

  renderCancelBtn() {
    return (
      <Col xs={24} sm={24} md={8} lg={8}>
        <FormItem>
          <Button loading={this.state.loading} onClick={this.gotoList} size="large" style={{ width: '100%', borderRadius: 0 }}>
            {I18N.get('from.CVoteForm.button.cancel')}
          </Button>
        </FormItem>
      </Col>
    )
  }

  renderSaveDraftBtn() {
    const { edit, data } = this.props
    const showButton = !edit || _.get(data, 'status') === CVOTE_STATUS.DRAFT

    return showButton && (
      <Col xs={24} sm={24} md={8} lg={8}>
        <FormItem>
          <Button loading={this.state.loading} size="large" onClick={this.saveDraft} className="d_btn">
            {I18N.get('from.CVoteForm.button.saveDraft')}
          </Button>
        </FormItem>
      </Col>
    )
  }

  renderSaveBtn() {
    const { edit, data } = this.props
    const btnText = edit && data.published ? I18N.get('from.CVoteForm.button.saveChanges') : I18N.get('from.CVoteForm.button.saveAndPublish')
    return (
      <Col xs={24} sm={24} md={8} lg={8}>
        <FormItem>
          <Button loading={this.state.loading} size="large" type="ebp" htmlType="submit" className="d_btn">
            {btnText}
          </Button>
        </FormItem>
      </Col>
    )
  }

  finishClick(id) {
    Modal.confirm({
      title: I18N.get('from.CVoteForm.modal.title'),
      content: '',
      okText: I18N.get('from.CVoteForm.modal.confirm'),
      okType: 'danger',
      cancelText: I18N.get('from.CVoteForm.modal.cancel'),
      onOk: () => {
        this.ord_loading(true)
        this.props.finishCVote({
          id,
        }).then(() => {
          message.success(I18N.get('from.CVoteForm.message.proposal.update.success'))
          this.ord_loading(false)
          this.gotoList()
        }).catch((e) => {
          message.error(e.message)
          this.ord_loading(false)
        })
      },
      onCancel() {
      },
    })
  }
}

export default Form.create()(C)
