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

import { Container, Title, TabPaneInner, Note, NoteHighlight, TabText } from './style'

const FormItem = Form.Item
const { TabPane } = Tabs

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
    <DraftEditor contentType={_.get(data, 'contentType')} />
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

const activeKeys = ['abstract', 'goal', 'motivation', 'relevance', 'budget', 'plan']

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      persist: true,
      loading: false,
      activeKeyNum: 0,
    }

    this.user = this.props.user
  }

  componentDidMount = () => {
    this.intervalId = setInterval(this.saveDraft, 5000)
  }

  componentWillUnmount() {
    clearInterval(this.intervalId)
  }

  ord_loading(f = false) {
    this.setState({ loading: f })
  }

  publishCVote = async (e) => {
    e.preventDefault()
    const { edit, form, updateCVote, onEdit, suggestionId } = this.props

    form.validateFields(async (err, values) => {
      if (err) {
        // mark error keys
        this.setState({ errKeys: _.keys(err)})
        return
      }
      const { title, abstract, goal, motivation, relevance, budget, plan } = values
      const param = {
        _id: edit,
        title,
        abstract: formatValue(abstract),
        goal: formatValue(goal),
        motivation: formatValue(motivation),
        relevance: formatValue(relevance),
        budget: formatValue(budget),
        plan: formatValue(plan),
        published: true,
      }
      if (suggestionId) param.suggestionId = suggestionId

      this.ord_loading(true)
      try {
        await updateCVote(param)
        this.ord_loading(false)
        await onEdit()
        message.success(I18N.get('from.CVoteForm.message.updated.success'))
      } catch (error) {
        message.error(error.message)
        this.ord_loading(false)
      }
    })
  }

  saveDraft = async (ifShowMsg = false) => {
    const { edit, form, updateDraft, suggestionId } = this.props

    const values = form.getFieldsValue()
    const { title, abstract, goal, motivation, relevance, budget, plan } = values
    const param = {
      _id: edit,
      title,
    }
    if (suggestionId) param.suggestionId = suggestionId

    if (!_.isEmpty(transform(abstract))) param.abstract = formatValue(abstract)
    if (!_.isEmpty(transform(goal))) param.goal = formatValue(goal)
    if (!_.isEmpty(transform(motivation))) param.motivation = formatValue(motivation)
    if (!_.isEmpty(transform(relevance))) param.relevance = formatValue(relevance)
    if (!_.isEmpty(transform(budget))) param.budget = formatValue(budget)
    if (!_.isEmpty(transform(plan))) param.plan = formatValue(plan)

    try {
      await updateDraft(param)
      if (ifShowMsg) message.success(I18N.get('from.CVoteForm.message.updated.success'))
    } catch (error) {
      message.error(error.message)
    }
  }

  saveDraftWithMsg = () => this.saveDraft(true)

  getInputProps(data) {
    const { edit } = this.props
    const { getFieldDecorator } = this.props.form

    const title_fn = getFieldDecorator('title', {
      rules: [{ required: true, message: I18N.get('proposal.form.error.required') }],
      initialValue: edit ? data.title : _.get(data, 'title', ''),
    })
    const title_el = (
      <Input size="large" type="text" />
    )

    const abstract = renderRichEditor(data, 'abstract', getFieldDecorator, 200)
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
    const formItemLayout = {
      labelCol: {
        span: 2,
      },
      wrapperCol: {
        span: 18,
      },
      colon: false,
    }
    const { activeKeyNum } = this.state
    const activeKey = activeKeys[activeKeyNum]
    const continueBtn = (activeKeyNum !== activeKeys.length - 1) && (
      <Row gutter={8} type="flex" justify="center">
        {this.renderContinueBtn()}
      </Row>
    )
    return (
      <Container>
        <Form onSubmit={this.publishCVote}>
          <Title className="komu-a cr-title-with-icon ">
            {this.props.header || I18N.get('from.CVoteForm.button.add')}
          </Title>
          <FormItem label={`${I18N.get('proposal.fields.title')}*`} {...formItemLayout}>
            {formProps.title}
          </FormItem>
          <Tabs animated={false} tabBarGutter={5} activeKey={activeKey} onChange={this.onTabChange}>
            <TabPane tab={this.renderTabText('abstract')} key="abstract">
              <TabPaneInner>
                <Note>{I18N.get('proposal.form.note.abstract')}</Note>
                <FormItem>{formProps.abstract}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText('goal')} key="goal">
              <TabPaneInner>
                <Note>{I18N.get('proposal.form.note.goal')}</Note>
                <FormItem>{formProps.goal}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText('motivation')} key="motivation">
              <TabPaneInner>
                <Note>
                  {I18N.get('proposal.form.note.motivation')}
                  <NoteHighlight> {I18N.get('proposal.form.note.motivationHighlight')}</NoteHighlight>
                </Note>
                <FormItem>{formProps.motivation}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText('relevance')} key="relevance">
              <TabPaneInner>
                <Note>{I18N.get('proposal.form.note.relevance')}</Note>
                <FormItem>{formProps.relevance}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText('budget')} key="budget">
              <TabPaneInner>
                <Note>{I18N.get('proposal.form.note.budget')}</Note>
                <FormItem>{formProps.budget}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText('plan')} key="plan">
              <TabPaneInner>
                <Note>{I18N.get('proposal.form.note.plan')}</Note>
                <FormItem>{formProps.plan}</FormItem>
              </TabPaneInner>
            </TabPane>
          </Tabs>

          {continueBtn}

          <Row gutter={8} type="flex" justify="center">
            {this.renderCancelBtn()}
            {this.renderSaveDraftBtn()}
            {this.renderSaveBtn()}
          </Row>
        </Form>
      </Container>
    )
  }

  renderTabText(key) {
    const { errKeys } = this.state
    const fullText = `${I18N.get(`proposal.fields.${key}`)}*`
    const hasErr = _.includes(errKeys, key)
    return (
      <TabText hasErr={hasErr}>{fullText}</TabText>
    )
  }

  onTabChange = (activeKey) => {
    const activeKeyNum = activeKeys.indexOf(activeKey)
    this.setState({ activeKeyNum })
  }

  gotoNextTab = () => {
    const currentKeyNum = this.state.activeKeyNum
    this.setState({ activeKeyNum: currentKeyNum + 1 })
  }

  gotoList = () => {
    this.props.history.push('/proposals')
  }

  renderContinueBtn() {
    return (
      <FormItem>
        <Button onClick={this.gotoNextTab} className="cr-btn cr-btn-black" style={{ marginBottom: 10 }}>
          {I18N.get('from.CVoteForm.button.continue')}
        </Button>
      </FormItem>
    )
  }

  renderCancelBtn() {
    return (
      <FormItem>
        <Button onClick={this.props.onCancel} className="cr-btn cr-btn-default" style={{ marginRight: 10 }}>
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
        <Button loading={this.state.loading} className="cr-btn cr-btn-primary" onClick={this.saveDraftWithMsg} style={{ marginRight: 10 }}>
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
