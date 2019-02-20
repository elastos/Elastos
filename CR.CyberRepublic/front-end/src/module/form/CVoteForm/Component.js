import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Form, Icon, Input, Button, Select, Row, Col, message, Steps, Modal,
} from 'antd'
import I18N from '@/I18N'
import _ from 'lodash'
import { LANGUAGES } from '@/config/constant'

import './style.scss'

const FormItem = Form.Item
const { TextArea } = Input

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      persist: true,
      loading: false,
      language: LANGUAGES.english, // language for this specifc form only
    }

    this.user = this.props.user
  }

  ord_loading(f = false) {
    this.setState({ loading: f })
  }

  handleSubmit = async (e, fields = {}) => {
    e.preventDefault()

    this.props.form.validateFields(async (err, values) => {
      if (!err) {
        const param = {
          title: values.title,
          type: values.type,
          notes: values.notes,
          motionId: values.motionId,
          isConflict: values.isConflict,
          proposedBy: values.proposedBy,
          content: values.content,
          published: true,
          ...fields,
        }

        this.ord_loading(true)
        if (this.props.edit) {
          try {
            param._id = this.props.edit
            await this.props.updateCVote(param)
            message.success(I18N.get('from.CVoteForm.message.updated.success'))
            this.ord_loading(false)
            this.gotoList()
          } catch (error) {
            message.error(error.message)
            this.ord_loading(false)
          }
        } else {
          try {
            await this.props.createCVote(param)
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
    const fullName = `${this.user.profile.firstName} ${this.user.profile.lastName}`
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
      initialValue: edit ? parseInt(data.type, 10) : '',
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

    const proposedBy_fn = getFieldDecorator('proposedBy', {
      rules: [{ required: true }],
      initialValue: edit ? data.proposedBy : fullName,
    })
    const proposedBy_el = (
      <Select size="large">
        {/* <Select.Option key={-1} value={-1}>please select</Select.Option> */}
        {
          _.map(s.voter, (item, i) => (
            <Select.Option key={i} value={item.value}>{item.value}</Select.Option>
          ))
        }
      </Select>
    )

    const motionId_fn = getFieldDecorator('motionId', {
      initialValue: edit ? data.motionId : '',
    })
    const motionId_el = (
      <Input size="large" type="text" />
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
      proposedBy: proposedBy_fn(proposedBy_el),
      motionId: motionId_fn(motionId_el),
      isConflict: isConflict_fn(isConflict_el),
      notes: notes_fn(notes_el),
    }
  }

  togglePersist() {
    this.setState({ persist: !this.state.persist })
  }

  ord_render() {
    const { edit, data, canManage } = this.props
    let p = null
    if (!canManage || (edit && !data)) {
      return null
    }
    if (edit) {
      p = this.getInputProps(data)
    } else {
      p = this.getInputProps()
    }
    const s = this.props.static
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
        <FormItem label={I18N.get('from.CVoteForm.label.proposedby')} {...formItemLayout}>{p.proposedBy}</FormItem>

        <FormItem style={{ marginBottom: '30px' }} label={I18N.get('from.CVoteForm.label.motion')} help={I18N.get('from.CVoteForm.label.motion.help')} {...formItemLayout}>{p.motionId}</FormItem>

        <FormItem style={{ marginBottom: '12px' }} label={I18N.get('from.CVoteForm.label.conflict')} help={I18N.get('from.CVoteForm.label.conflict.help')} {...formItemLayout}>{p.isConflict}</FormItem>
        <FormItem label={I18N.get('from.CVoteForm.label.note')} {...formItemLayout}>{p.notes}</FormItem>
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
          <Button loading={this.state.loading} onClick={this.gotoList} size="large" className="d_btn">
            {I18N.get('from.CVoteForm.button.cancel')}
          </Button>
        </FormItem>
      </Col>
    )
  }

  renderSaveDraftBtn() {
    const { edit } = this.props

    if (edit) return null

    return (
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
