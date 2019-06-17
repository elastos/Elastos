import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Form, Input, Button, Row, message, Modal, Tabs,
} from 'antd'
import I18N from '@/I18N'
import _ from 'lodash'
import { CVOTE_STATUS } from '@/constant'
import { convertToRaw } from 'draft-js'
import DraftEditor from '@/module/common/DraftEditor'

// if using webpack
import 'medium-draft/lib/index.css'

import { Container, Title } from './style'

const FormItem = Form.Item
const { TabPane } = Tabs

const renderRichEditor = (data, key, getFieldDecorator) => {
  const content = _.get(data, key, '')
  const content_fn = getFieldDecorator(key, {
    rules: [{ required: true }],
    initialValue: content,
  })
  const content_el = (
    <DraftEditor content={content} contentType={_.get(data, 'contentType')} />
  )
  return content_fn(content_el)
}


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
    const { email, profile } = this.user
    const fullName = `${profile.firstName} ${profile.lastName}`
    const { edit, form, updateCVote, createCVote, onCreated, onEdit, suggestionId } = this.props

    form.validateFields(async (err, values) => {
      if (err) return
      const { title, notes, abstract, goal, motivation, relevance, budget, plan } = values
      const param = {
        title,
        notes,
        abstract: JSON.stringify(convertToRaw(abstract.getCurrentContent())),
        goal: JSON.stringify(convertToRaw(goal.getCurrentContent())),
        motivation: JSON.stringify(convertToRaw(motivation.getCurrentContent())),
        relevance: JSON.stringify(convertToRaw(relevance.getCurrentContent())),
        budget: JSON.stringify(convertToRaw(budget.getCurrentContent())),
        plan: JSON.stringify(convertToRaw(plan.getCurrentContent())),
        published: true,
        ...fields,
      }
      if (!edit) param.proposedBy = fullName
      if (!edit) param.proposedByEmail = email
      if (suggestionId) param.suggestionId = suggestionId
      console.log('form values: ', this.user, values, param)

      this.ord_loading(true)
      if (edit) {
        try {
          param._id = edit
          await updateCVote(param)
          this.ord_loading(false)
          await onEdit()
          window.location.reload()
          message.success(I18N.get('from.CVoteForm.message.updated.success'))
        } catch (error) {
          message.error(error.message)
          this.ord_loading(false)
        }
      } else {
        try {
          await createCVote(param)
          this.ord_loading(false)
          await onCreated()
          message.success(I18N.get('from.CVoteForm.message.create.success'))
        } catch (error) {
          message.error(error.message)
          this.ord_loading(false)
        }
      }
    })
  }

  getInputProps(data) {
    const { edit } = this.props
    const { getFieldDecorator } = this.props.form

    const title_fn = getFieldDecorator('title', {
      rules: [{ required: true }],
      initialValue: edit ? data.title : _.get(data, 'title', ''),
    })
    const title_el = (
      <Input size="large" type="text" />
    )

    const abstract = renderRichEditor(data, 'abstract', getFieldDecorator)
    const goal = renderRichEditor(data, 'goal', getFieldDecorator)
    const motivation = renderRichEditor(data, 'motivation', getFieldDecorator)
    const relevance = renderRichEditor(data, 'relevance', getFieldDecorator)
    const budget = renderRichEditor(data, 'budget', getFieldDecorator)
    const plan = renderRichEditor(data, 'plan', getFieldDecorator)

    return {
      title: title_fn(title_el),
      abstract,
      goal,
      motivation,
      relevance,
      budget,
      plan,
    }
  }

  ord_render() {
    const { edit, data, canManage } = this.props
    if (!canManage || (edit && !data)) {
      return null
    }
    const formProps = this.getInputProps(data)

    return (
      <Container>
        <Form onSubmit={this.handleSubmit}>
          <Title>
            {this.props.header || I18N.get('from.CVoteForm.button.add')}
          </Title>
          <Tabs defaultActiveKey="abstract">
            <TabPane tab="title" key="title">
              <FormItem>{formProps.title}</FormItem>
            </TabPane>
            <TabPane tab="abstract" key="abstract">
              <FormItem>{formProps.abstract}</FormItem>
            </TabPane>
            <TabPane tab="goal" key="goal">
              <FormItem>{formProps.goal}</FormItem>
            </TabPane>
            <TabPane tab="motivation" key="motivation">
              <FormItem>{formProps.motivation}</FormItem>
            </TabPane>
            <TabPane tab="relevance" key="relevance">
              <FormItem>{formProps.relevance}</FormItem>
            </TabPane>
            <TabPane tab="budget" key="budget">
              <FormItem>{formProps.budget}</FormItem>
            </TabPane>
            <TabPane tab="plan" key="plan">
              <FormItem>{formProps.plan}</FormItem>
            </TabPane>
          </Tabs>

          <Row gutter={8} type="flex" justify="center">
            {this.renderCancelBtn()}
            {this.renderSaveDraftBtn()}
            {this.renderSaveBtn()}
          </Row>
        </Form>
      </Container>
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
      <FormItem>
        <Button loading={this.state.loading} onClick={this.props.onCancel} className="cr-btn cr-btn-default" style={{ marginRight: 10 }}>
          {I18N.get('from.CVoteForm.button.cancel')}
        </Button>
      </FormItem>
    )
  }

  renderSaveDraftBtn() {
    const { edit, data } = this.props
    const showButton = !edit || _.get(data, 'status') === CVOTE_STATUS.DRAFT

    return showButton && (
      <FormItem>
        <Button loading={this.state.loading} className="cr-btn cr-btn-primary" onClick={this.saveDraft} style={{ marginRight: 10 }}>
          {I18N.get('from.CVoteForm.button.saveDraft')}
        </Button>
      </FormItem>
    )
  }

  renderSaveBtn() {
    const { edit, data } = this.props
    const btnText = edit && data.published ? I18N.get('from.CVoteForm.button.saveChanges') : I18N.get('from.CVoteForm.button.saveAndPublish')
    return (
      <FormItem>
        <Button loading={this.state.loading} className="cr-btn cr-btn-primary" htmlType="submit">
          {btnText}
        </Button>
      </FormItem>
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
