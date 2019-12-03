import React from 'react'
import { Form, Input, Button, Row, Tabs, Radio } from 'antd'
import _ from 'lodash'
import BaseComponent from '@/model/BaseComponent'
import I18N from '@/I18N'
import { ABSTRACT_MAX_WORDS } from '@/constant'
import CircularProgressbar from '@/module/common/CircularProgressbar'
import CodeMirrorEditor from '@/module/common/CodeMirrorEditor'
import PaymentSchedule from './PaymentSchedule'
import ImplementationPlan from './ImplementationPlan'

import { Container, TabPaneInner, Note, TabText, CirContainer } from './style'

const FormItem = Form.Item
const { TabPane } = Tabs

const WORD_LIMIT = ABSTRACT_MAX_WORDS

const TAB_KEYS = [
  'type',
  'abstract',
  'goal',
  'motivation',
  'plan',
  'relevance',
  'budget'
]

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.timer = -1
    this.state = {
      loading: false,
      activeKey: TAB_KEYS[0],
      errorKeys: {}
    }
  }

  componentDidMount() {
    this.timer = setInterval(() => {
      this.handleSaveDraft()
    }, 5000)
  }

  componentWillUnmount() {
    clearInterval(this.timer)
  }

  getActiveKey(key) {
    if (!TAB_KEYS.includes(key)) return this.state.activeKey
    return key
  }

  handleSubmit = e => {
    e.preventDefault()
    const { onSubmit, form } = this.props
    this.setState({ loading: true })
    form.validateFields(async (err, values) => {
      if (err) {
        this.setState({
          loading: false,
          errorKeys: err,
          activeKey: this.getActiveKey(Object.keys(err)[0])
        })
        return
      }
      await onSubmit(values)
      this.setState({ loading: false })
    })
  }

  handleSaveDraft = () => {
    const { form } = this.props
    if (this.props.onSaveDraft) {
      const values = form.getFieldsValue()
      this.props.onSaveDraft(values)
    }
  }

  handleContinue = e => {
    e.preventDefault()
    const { form } = this.props
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

  getTitleInput() {
    const { initialValues = {} } = this.props
    const { getFieldDecorator } = this.props.form

    return getFieldDecorator('title', {
      rules: [
        { required: true, message: I18N.get('suggestion.form.error.required') }
      ],
      initialValue: initialValues.title
    })(<Input size="large" type="text" />)
  }

  onTextareaChange = activeKey => {
    const { form } = this.props
    const err = form.getFieldError(activeKey)
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

  validateAbstract = (rule, value, cb) => {
    const { lang } = this.props
    let count = 0
    if (value) {
      const rs = value.replace(/\!\[image\]\(data:image\/.*\)/g, '')
      count = lang === 'en' ? rs.split(' ').length : rs.length
    }
    return count > WORD_LIMIT ? cb(true) : cb()
  }

  validatePlan = (rule, value, cb) => {
    if (value && _.isEmpty(value.teamInfo)) {
      return cb(true)
    } if (value && _.isEmpty(value.milestone)) {
      return cb(true)
    }
    return cb()
  }

  getTextarea(id) {
    const initialValues = _.isEmpty(this.props.initialValues)
      ? { type: '1' }
      : this.props.initialValues

    const { getFieldDecorator } = this.props.form
    const rules = [
      {
        required: true,
        message: I18N.get('suggestion.form.error.required')
      }
    ]
    if (id === 'abstract') {
      rules.push({
        message: I18N.get(`suggestion.form.error.limit${WORD_LIMIT}`),
        validator: this.validateAbstract
      })
    }
    if (
      id === 'plan' &&
      ((initialValues.plan && typeof initialValues.plan !== 'string') ||
        !initialValues.plan)
    ) {
      rules.push({
        message: I18N.get('suggestion.form.error.plan'),
        validator: this.validatePlan
      })
    }

    let rc
    if (id === 'type') {
      rc = (
        <Radio.Group>
          <Radio value="1">{I18N.get('suggestion.form.type.newMotion')}</Radio>
          <Radio value="2">
            {I18N.get('suggestion.form.type.motionAgainst')}
          </Radio>
          <Radio value="3">
            {I18N.get('suggestion.form.type.anythingElse')}
          </Radio>
        </Radio.Group>
      )
    } else if (
      id === 'plan' &&
      ((initialValues.plan && typeof initialValues.plan !== 'string') ||
        !initialValues.plan)
    ) {
      rc = (
        <ImplementationPlan
          initialValue={initialValues.plan}
          callback={this.onTextareaChange}
        />
      )
    } else if (
      id === 'budget' &&
      ((initialValues.budget && typeof initialValues.budget !== 'string') ||
        !initialValues.budget)
    ) {
      rc = (
        <PaymentSchedule
          initialValue={initialValues.budget}
          callback={this.onTextareaChange}
        />
      )
    } else {
      rc = (
        <CodeMirrorEditor
          callback={this.onTextareaChange}
          content={initialValues[id]}
          activeKey={id}
          name={id}
        />
      )
    }

    return getFieldDecorator(id, {
      rules,
      initialValue: initialValues[id]
    })(rc)
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

  renderWordLimit() {
    const { form, lang } = this.props
    const value = form.getFieldValue('abstract')
    let count = 0
    if (value) {
      const rs = value.replace(/\!\[image\]\(data:image\/.*\)/g, '')
      count = lang === 'en' ? rs.split(' ').length : rs.length
    }
    return (
      <CirContainer>
        <CircularProgressbar count={count} />
      </CirContainer>
    )
  }

  ord_render() {
    return (
      <Container>
        <Form onSubmit={this.handleSubmit}>
          <FormItem
            label={`${I18N.get('suggestion.form.fields.title')}*`}
            labelCol={{ span: 2 }}
            wrapperCol={{ span: 18 }}
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
            {TAB_KEYS.map(item => (
              <TabPane tab={this.renderTabText(item)} key={item}>
                <TabPaneInner>
                  <Note>{I18N.get(`suggestion.form.note.${item}`)}</Note>
                  <FormItem>{this.getTextarea(item)}</FormItem>
                  {item === 'abstract' ? this.renderWordLimit() : null}
                </TabPaneInner>
              </TabPane>
            ))}
          </Tabs>

          <Row
            gutter={8}
            type="flex"
            justify="center"
            style={{ marginBottom: '30px' }}
          >
            <Button
              onClick={this.handleContinue}
              className="cr-btn cr-btn-black"
              htmlType="button"
            >
              {I18N.get('suggestion.form.button.continue')}
            </Button>
          </Row>

          <Row gutter={8} type="flex" justify="center">
            <Button
              onClick={this.props.onCancel}
              className="cr-btn cr-btn-default"
              htmlType="button"
              style={{ marginRight: 10 }}
            >
              {I18N.get('suggestion.form.button.cancel')}
            </Button>
            <Button
              loading={this.state.loading}
              className="cr-btn cr-btn-primary"
              htmlType="submit"
            >
              {I18N.get('suggestion.form.button.save')}
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
