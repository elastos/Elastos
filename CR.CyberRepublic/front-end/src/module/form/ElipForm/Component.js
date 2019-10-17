import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Input, Button, Row, Radio, Tabs, Typography, Icon } from 'antd'
import I18N from '@/I18N'
import _ from 'lodash'
import CodeMirrorEditor from '@/module/common/CodeMirrorEditor'

import {
  Container,
  Title,
  TabPaneInner,
  Note,
  TabText,
  RadioCardLabel,
  RadioCardSpan
} from './style'

const { Paragraph } = Typography
const FormItem = Form.Item
const { TabPane } = Tabs

const TAB_KEYS = [
  'type',
  'abstract',
  'specifications',
  'motivation',
  'rationale',
  'backwardCompatibility',
  'referenceImplementation',
  'copyrightDomain'
]
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

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      loading: false,
      activeKey: TAB_KEYS[0],
      errorKeys: {}
    }
  }

  getActiveKey(key) {
    return !TAB_KEYS.includes(key) ? this.state.activeKey : key
  }

  handleSubmit = async e => {
    const { onSubmit, form } = this.props
    this.setState({ loading: true })
    e.preventDefault()
    form.validateFields(async (err, values) => {
      if (err) {
        return
      }

      onSubmit({
        title: values.title,
        type: values.type,
        abstract: values.abstract,
        specifications: values.specifications,
        motivation: values.motivation,
        rationale: values.rationale,
        backwardCompatibility: values.backwardCompatibility,
        referenceImplementation: values.referenceImplementation,
        copyrightDomain: values.copyrightDomain
      }).finally(() => this.state({ loading: false }))
    })
  }

  handleContinue = e => {
    const { form } = this.props
    e.preventDefault()

    form.validateFields((err, values) => {
      if (err) {
        this.setState({
          loading: false,
          errorKeys: err,
          activeKey: this.getActiveKey(Object.keys(err)[0])
        })
        return
      }

      const index = TAB_KEYS.findIndex(item => item === this.state.activeKey)
      if (index === TAB_KEYS.length - 1) {
        this.handleSubmit({ preventDefault: () => {} })
      } else {
        this.setState({ activeKey: TAB_KEYS[index + 1] })
      }
    })
  }

  onTextareaChange = activeKey => {
    const { form } = this.props
    const err = editorTransform(form.getFieldError(activeKey))
    const { errorKeys } = this.state
    if (err) {
      this.setState({
        errorKeys: Object.assign({}, errorKeys, { [activeKey]: err })
      })
    } else {
      const newState = Object.assign({}, errorKeys)
      delete newState[activeKey]
      this.setState({ errorKeys: newState })
    }
  }

  getTypeRadioGroup = key => {
    const { getFieldDecorator } = this.props.form
    const rules = [
      {
        required: true,
        message: I18N.get('elip.form.error.required')
      }
    ]
    return getFieldDecorator(key, {
      rules,
      initialValue: '1'
    })(<RadioCard radioKey={key} />)
  }

  getTextarea(id) {
    const { data = {} } = this.props
    const { getFieldDecorator } = this.props.form

    const rules = [
      {
        required: true,
        message: I18N.get('elip.form.error.required')
      }
    ]
    return getFieldDecorator(id, {
      rules,
      initialValue: data[id]
    })(
      <CodeMirrorEditor
        content={data[id]}
        callback={this.onTextareaChange}
        activeKey={id}
      />
    )
  }

  renderTabText(id) {
    const hasError = _.has(this.state.errorKeys, id)
    return (
      <TabText hasErr={hasError}>{`${I18N.get(`elip.fields.${id}`)}*`}</TabText>
    )
  }

  ord_render() {
    const { form, data, onCancel } = this.props
    const { getFieldDecorator } = form

    return (
      <Container>
        <Form onSubmit={this.handleSubmit}>
          <Title className="komu-a cr-title-with-icon ">
            {data
              ? `${I18N.get('elip.button.edit')}`
              : I18N.get('elip.button.add')}
          </Title>
          <FormItem
            label={`${I18N.get('elip.fields.title')}*`}
            labelCol={{ span: 2 }}
            wrapperCol={{ span: 18 }}
            colon={false}
          >
            {getFieldDecorator('title', {
              rules: [
                {
                  required: true,
                  message: I18N.get('elip.form.error.required')
                }
              ],
              initialValue: data && data.title ? data.title : ''
            })(<Input size="large" type="text" />)}
          </FormItem>
          <Tabs
            animated={false}
            tabBarGutter={TAB_KEYS.length}
            activeKey={this.state.activeKey}
            onChange={this.onTabChange}
          >
            <TabPane tab={this.renderTabText(TAB_KEYS[0])} key={TAB_KEYS[0]}>
              <TabPaneInner>
                <Note>{I18N.get(`elip.form.note.${TAB_KEYS[0]}`)}</Note>
                <FormItem>{this.getTypeRadioGroup(TAB_KEYS[0])}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText(TAB_KEYS[1])} key={TAB_KEYS[1]}>
              <TabPaneInner>
                <Note>{I18N.get(`elip.form.note.${TAB_KEYS[1]}`)}</Note>
                <FormItem>{this.getTextarea(TAB_KEYS[1])}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText(TAB_KEYS[2])} key={TAB_KEYS[2]}>
              <TabPaneInner>
                <Note>{I18N.get(`elip.form.note.${TAB_KEYS[2]}`)}</Note>
                <FormItem>{this.getTextarea(TAB_KEYS[2])}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText(TAB_KEYS[3])} key={TAB_KEYS[3]}>
              <TabPaneInner>
                <Note>{I18N.get(`elip.form.note.${TAB_KEYS[3]}`)}</Note>
                <FormItem>{this.getTextarea(TAB_KEYS[3])}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText(TAB_KEYS[4])} key={TAB_KEYS[4]}>
              <TabPaneInner>
                <Note>{I18N.get(`elip.form.note.${TAB_KEYS[4]}`)}</Note>
                <FormItem>{this.getTextarea(TAB_KEYS[4])}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText(TAB_KEYS[5])} key={TAB_KEYS[5]}>
              <TabPaneInner>
                <Note>{I18N.get(`elip.form.note.${TAB_KEYS[5]}`)}</Note>
                <FormItem>{this.getTextarea(TAB_KEYS[5])}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText(TAB_KEYS[6])} key={TAB_KEYS[6]}>
              <TabPaneInner>
                <Note>{I18N.get(`elip.form.note.${TAB_KEYS[6]}`)}</Note>
                <FormItem>{this.getTextarea(TAB_KEYS[6])}</FormItem>
              </TabPaneInner>
            </TabPane>
            <TabPane tab={this.renderTabText(TAB_KEYS[7])} key={TAB_KEYS[7]}>
              <TabPaneInner>
                <Note>{I18N.get(`elip.form.note.${TAB_KEYS[7]}`)}</Note>
                <FormItem>{this.getTextarea(TAB_KEYS[7])}</FormItem>
              </TabPaneInner>
            </TabPane>
          </Tabs>

          <Row
            gutter={8}
            type="flex"
            justify="center"
            style={{ marginBottom: '30px' }}
          >
            <Button
              loading={this.state.loading}
              onClick={this.handleContinue}
              className="cr-btn cr-btn-black"
              htmlType="button"
            >
              {I18N.get('suggestion.form.button.continue')}
            </Button>
          </Row>

          <Row gutter={8} type="flex" justify="center">
            <Button
              onClick={onCancel}
              className="cr-btn cr-btn-default"
              style={{ marginRight: 10 }}
            >
              {I18N.get('elip.button.cancel')}
            </Button>

            <Button
              loading={this.state.loading}
              className="cr-btn cr-btn-primary"
              htmlType="submit"
            >
              {I18N.get('elip.button.submit')}
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

class RadioCard extends BaseComponent {
  constructor(props) {
    super(props)
    this.state = {
      data: this.props && this.props.value ? this.props.value : '0'
    }
    this.handleChange = this.handleChange.bind(this)
  }

  renderCheck(value) {
    return (
      <RadioCardSpan className="radio-card-flex">
        {this.state.data === value ? (
          <Icon type="check" className="radio-card-check" />
        ) : (
          ''
        )}
      </RadioCardSpan>
    )
  }

  renderContent(id) {
    const contentStr = I18N.get(`elip.form.type.${id}`)
    const contentArr = _.split(contentStr, '<br />')
    const content = _.map(contentArr, (v, n) => {
      return <Paragraph key={n}>{v}</Paragraph>
    })
    return (
      <Typography>
        <RadioCardSpan style={{ fontWeight: 'bold' }}>
          {I18N.get(`elip.form.type.${id}Title`)}
        </RadioCardSpan>
        {content}
      </Typography>
    )
  }

  handleChange(evt) {
    this.setState({ data: evt.target.value })
  }

  ord_render() {
    const { radioKey } = this.props
    return (
      <div>
        <div>
          <Radio.Group defaultValue="a">
            <RadioCardLabel
              className="radio-card-wrapper"
              style={{ width: 238.8, height: 350 }}
            >
              <RadioCardSpan>
                <Input
                  type="radio"
                  name={radioKey}
                  value="1"
                  style={{ display: 'none' }}
                  onChange={this.handleChange}
                />
              </RadioCardSpan>
              <RadioCardSpan>
                <Paragraph>{this.renderContent('standard')}</Paragraph>
                {this.renderCheck('1')}
              </RadioCardSpan>
            </RadioCardLabel>
            <RadioCardLabel
              className="radio-card-wrapper"
              style={{ width: 238.8, height: 350 }}
            >
              <RadioCardSpan>
                <Input
                  type="radio"
                  name={radioKey}
                  value="2"
                  style={{ display: 'none' }}
                  onChange={this.handleChange}
                />
              </RadioCardSpan>
              <RadioCardSpan>
                <Paragraph>{this.renderContent('information')}</Paragraph>
                {this.renderCheck('2')}
              </RadioCardSpan>
            </RadioCardLabel>
            <RadioCardLabel
              className="radio-card-wrapper"
              style={{ width: 477.6, height: 350 }}
            >
              <RadioCardSpan>
                <Input
                  type="radio"
                  name={radioKey}
                  value="3"
                  style={{ display: 'none' }}
                  onChange={this.handleChange}
                />
              </RadioCardSpan>
              <RadioCardSpan>
                <Paragraph>{this.renderContent('process')}</Paragraph>
                {this.renderCheck('3')}
              </RadioCardSpan>
            </RadioCardLabel>
          </Radio.Group>
        </div>
      </div>
    )
  }
}

export default Form.create()(C)
