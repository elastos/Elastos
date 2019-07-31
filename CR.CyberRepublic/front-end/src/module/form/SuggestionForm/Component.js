import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Input, Button, Row, message, Modal, Tabs, Radio } from 'antd'
import I18N from '@/I18N'
import _ from 'lodash'
import { CVOTE_STATUS, CONTENT_TYPE } from '@/constant'
import { convertToRaw } from 'draft-js'
import DraftEditor from '@/module/common/DraftEditor'
import CircularProgressbar from '@/module/common/CircularProgressbar'

import 'medium-draft/lib/index.css'

import {
  Container,
  Title,
  TabPaneInner,
  Note,
  NoteHighlight,
  TabText,
  CirContainer
} from './style'

const FormItem = Form.Item
const { TabPane } = Tabs

const WORD_LIMIT = 200

const editorTransform = value => {
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
  constructor(props) {
    super(props)

    this.state = {
      loading: false,
      activeKey: 'abstract',
      errorKeys: {},
    }
  }

  handleSubmit = async e => {
    const { onSubmit, form } = this.props
    this.setState({ loading: true })

    e.preventDefault()
    form.validateFields((err, values) => {
      if (err) {
        this.setState({ loading: false, errorKeys: err, activeKey: Object.keys(err)[0] })
        return
      }

      onSubmit({
        title: values.title,
        abstract: formatValue(values.abstract),
        goal: formatValue(values.goal),
        motivation: formatValue(values.motivation),
        relevance: formatValue(values.relevance),
        budget: formatValue(values.budget),
        plan: formatValue(values.plan)
      }).finally(() => this.setState({ loading: false }))
    })
  }

  getTitleInput() {
    const { initialValues = {} } = this.props
    const { getFieldDecorator } = this.props.form

    return getFieldDecorator('title', {
      rules: [
        { required: true, message: I18N.get('suggestion.form.error.required') }
      ],
      initialValue: initialValues.title
    })(
      <Input size="large" type="text" />
    )
  }

  getTextarea(id) {
    const { initialValues = {} } = this.props
    const { getFieldDecorator } = this.props.form
    return getFieldDecorator(id, {
      rules: [
        {
          required: true,
          transform: editorTransform,
          message: I18N.get('suggestion.form.error.required')
        },
        {
          max: 200,
          transform: editorTransform,
          message: I18N.get('proposal.form.error.limit200')
        }
      ],
      validateTrigger: 'onSubmit',
      initialValue: initialValues[id],
    })(<DraftEditor contentType={CONTENT_TYPE.HTML} /* callback={callback} */ />)
  }

  renderTabText(id) {
    const hasError = _.has(this.state.errorKeys, id)
    return (
      <TabText hasErr={hasError}>
        {I18N.get(`suggestion.fields.${id}`)}
*
      </TabText>
    )
  }

  ord_render() {
    return (
      <Container>
        <Form onSubmit={this.handleSubmit}>
          <FormItem
            label={`${I18N.get('suggestion.form.fields.title')}*`}
            labelCol={{span: 2}}
            wrapperCol={{span: 18}}
            colon={false}
          >
            {this.getTitleInput()}
          </FormItem>

          <Tabs
            animated={false}
            tabBarGutter={5}
            activeKey={this.state.activeKey}
            onChange={this.onTabChange}
          >
            <TabPane tab={this.renderTabText('abstract')} key="abstract">
              <TabPaneInner>
                <Note>{I18N.get('suggestion.form.note.abstract')}</Note>
                <FormItem>{this.getTextarea('abstract')}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText('goal')} key="goal">
              <TabPaneInner>
                <Note>{I18N.get('suggestion.form.note.goal')}</Note>
                <FormItem>{this.getTextarea('goal')}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText('motivation')} key="motivation">
              <TabPaneInner>
                <Note>{I18N.get('suggestion.form.note.motivation')}</Note>
                <FormItem>{this.getTextarea('motivation')}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText('plan')} key="plan">
              <TabPaneInner>
                <Note>{I18N.get('suggestion.form.note.plan')}</Note>
                <FormItem>{this.getTextarea('plan')}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText('relevance')} key="relevance">
              <TabPaneInner>
                <Note>{I18N.get('suggestion.form.note.relevance')}</Note>
                <FormItem>{this.getTextarea('relevance')}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText('budget')} key="budget">
              <TabPaneInner>
                <Note>{I18N.get('suggestion.form.note.budget')}</Note>
                <FormItem>{this.getTextarea('budget')}</FormItem>
              </TabPaneInner>
            </TabPane>
          </Tabs>

          <Row gutter={8} type="flex" justify="center">
            <Button
              loading={this.state.loading}
              className="cr-btn cr-btn-primary"
              htmlType="submit"
            >
              {I18N.get('from.CVoteForm.button.saveChanges')}
            </Button>
            <Button
              onClick={this.props.onCancel}
              className="cr-btn cr-btn-default"
              style={{ marginRight: 10 }}
            >
              {I18N.get('from.CVoteForm.button.cancel')}
            </Button>
          </Row>
        </Form>
      </Container>
    )
  }

  onTabChange = activeKey => {
    this.setState({ activeKey })
  }
}

export default Form.create()(C)
