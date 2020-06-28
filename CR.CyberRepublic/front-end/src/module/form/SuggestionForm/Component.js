import React from 'react'
import { Form, Input, Button, Row, Tabs, Radio, message } from 'antd'
import _ from 'lodash'
import BaseComponent from '@/model/BaseComponent'
import I18N from '@/I18N'
import { ABSTRACT_MAX_WORDS } from '@/constant'
import CircularProgressbar from '@/module/common/CircularProgressbar'
import CodeMirrorEditor from '@/module/common/CodeMirrorEditor'
import PaymentSchedule from './PaymentSchedule'
import ImplementationPlan from './ImplementationPlan'
import { wordCounter } from '@/util'
import { SUGGESTION_BUDGET_TYPE } from '@/constant'
import { Container, TabPaneInner, Note, TabText, CirContainer } from './style'

const FormItem = Form.Item
const { TabPane } = Tabs

const WORD_LIMIT = ABSTRACT_MAX_WORDS

const TAB_KEYS = [
  'type',
  'abstract',
  'motivation',
  'goal',
  'plan',
  'relevance',
  'budget'
]
const { ADVANCE, COMPLETION } = SUGGESTION_BUDGET_TYPE

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.timer = -1
    this.state = {
      loading: false,
      activeKey: TAB_KEYS[0],
      errorKeys: {}
    }
    const sugg = props.initialValues
    if (
      sugg &&
      sugg.plan &&
      typeof sugg.plan !== 'string' &&
      sugg.plan.milestone
    ) {
      sessionStorage.setItem(
        'plan-milestone',
        JSON.stringify(sugg.plan.milestone)
      )
    }
  }

  componentDidMount() {
    this.timer = setInterval(() => {
      this.handleSaveDraft()
    }, 5000)
  }

  componentWillUnmount() {
    clearInterval(this.timer)
    sessionStorage.removeItem('plan-milestone')
  }

  getActiveKey(key) {
    if (!TAB_KEYS.includes(key)) return this.state.activeKey
    return key
  }

  handleSave = (e, callback) => {
    e.preventDefault()
    const { form } = this.props
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

      const milestone = _.get(values, 'plan.milestone')
      const amount = _.get(values, 'budget.budgetAmount')
      const pItems = _.get(values, 'budget.paymentItems')

      const sum = pItems.reduce((sum, item) => {
        return (sum += Number(item.amount))
      }, 0)

      if (Number(amount) !== sum) {
        this.setState({ loading: false })
        message.error(I18N.get('suggestion.form.error.notEqual'))
        return
      }

      const initiation = pItems.filter(
        (item) => item.type === ADVANCE && item.milestoneKey === '0'
      )
      const completion = pItems.filter((item) => {
        return (
          item.type === COMPLETION &&
          item.milestoneKey === (milestone.length - 1).toString()
        )
      })
      if (
        milestone.length !== pItems.length ||
        initiation.length > 1 ||
        completion.length !== 1
      ) {
        this.setState({ loading: false })
        message.error(I18N.get('suggestion.form.error.payment'))
        return
      }

      const budget = _.get(values, 'budget')
      // exclude old suggestion data
      if (budget && typeof budget !== 'string') {
        values.budget = budget.paymentItems
        values.budgetAmount = Number(budget.budgetAmount)
        values.elaAddress = budget.elaAddress
      }

      await callback(values)
      this.setState({ loading: false })
    })
  }

  handleSubmit = (e) => {
    const { onSubmit } = this.props
    this.handleSave(e, onSubmit)
  }

  handleEditSaveDraft = (e) => {
    const { onSaveDraft } = this.props
    this.handleSave(e, onSaveDraft)
  }

  handleSaveDraft = () => {
    const { isEditMode, form } = this.props
    if (!isEditMode && this.props.onSaveDraft) {
      const values = form.getFieldsValue()
      const budget = form.getFieldValue('budget')
      if (budget) {
        values.budget = budget.paymentItems
        values.budgetAmount = budget.budgetAmount && Number(budget.budgetAmount)
        values.elaAddress = budget.elaAddress
      }
      this.props.onSaveDraft(values)
    }
  }

  handleContinue = (e) => {
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
      const index = TAB_KEYS.findIndex((item) => item === this.state.activeKey)
      if (index !== TAB_KEYS.length - 1) {
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

  onTextareaChange = (activeKey) => {
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
    let count = 0
    if (value) {
      const rs = value.replace(/\!\[image\]\(data:image\/.*\)/g, '')
      count = wordCounter(rs)
    }
    return count > WORD_LIMIT ? cb(true) : cb()
  }

  validatePlan = (rule, value, cb) => {
    if (value && _.isEmpty(value.teamInfo)) {
      return cb(I18N.get('suggestion.form.error.team'))
    }
    if (value && _.isEmpty(value.milestone)) {
      return cb(I18N.get('suggestion.form.error.milestones'))
    }
    return cb()
  }

  validateAmount = (value) => {
    const reg = /^(0|[1-9][0-9]*)(\.[0-9]*)?$/
    return (!isNaN(value) && reg.test(value)) || value === '' ? true : false
  }

  validateAddress = (value) => {
    const reg = /^[E8][a-zA-Z0-9]{33}$/
    return reg.test(value)
  }

  validateBudget = (rule, value, cb) => {
    const amount = _.get(value, 'budgetAmount')
    const address = _.get(value, 'elaAddress')
    const pItems = _.get(value, 'paymentItems')

    if (!this.validateAmount(amount)) {
      return cb(I18N.get('suggestion.form.error.isNaN'))
    }
    if (!this.validateAddress(address)) {
      return cb(I18N.get('suggestion.form.error.elaAddress'))
    }
    if (_.isEmpty(pItems)) {
      return cb(I18N.get('suggestion.form.error.schedule'))
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
    if (id === 'type') {
      return getFieldDecorator(id, {
        rules,
        initialValue: initialValues[id]
      })(
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
    }

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
        validator: this.validatePlan
      })

      return getFieldDecorator('plan', {
        rules,
        initialValue: initialValues.plan
      })(
        <ImplementationPlan
          initialValue={initialValues.plan}
          callback={this.onTextareaChange}
        />
      )
    }

    if (id === 'budget') {
      let initialBudget = {}
      if (initialValues.budget && typeof initialValues.budget !== 'string') {
        initialBudget = initialValues.budget && {
          budgetAmount: initialValues.budgetAmount,
          elaAddress: initialValues.elaAddress,
          paymentItems: initialValues.budget
        }
      } else {
        initialBudget = {
          budgetAmount: initialValues.budget,
          elaAddress: '',
          paymentItems: []
        }
      }

      rules.push({
        validator: this.validateBudget
      })

      return getFieldDecorator('budget', {
        rules,
        initialValue: initialBudget
      })(
        <PaymentSchedule
          initialValue={initialBudget}
          callback={this.onTextareaChange}
        />
      )
    }

    return getFieldDecorator(id, {
      rules,
      initialValue: initialValues[id]
    })(
      <CodeMirrorEditor
        callback={this.onTextareaChange}
        content={initialValues[id]}
        activeKey={id}
        name={id}
      />
    )
  }

  renderTabText(id) {
    const hasError = _.has(this.state.errorKeys, id)
    return (
      <TabText hasErr={hasError}>
        {I18N.get(`suggestion.fields.${id}`)}*
      </TabText>
    )
  }

  renderWordLimit() {
    const { form } = this.props
    const value = form.getFieldValue('abstract')
    let count = 0
    if (value) {
      const rs = value.replace(/\!\[image\]\(data:image\/.*\)/g, '')
      count = wordCounter(rs)
    }
    return (
      <CirContainer>
        <CircularProgressbar count={count} />
      </CirContainer>
    )
  }

  hideContinue = () => {
    const { activeKey, errorKeys } = this.state
    const index = TAB_KEYS.findIndex((item) => item === activeKey)
    return _.isEmpty(errorKeys) && index === TAB_KEYS.length - 1
  }

  ord_render() {
    const { isEditMode } = this.props
    const saveDraftBtn = isEditMode && (
      <Button
        onClick={this.handleEditSaveDraft}
        className="cr-btn cr-btn-default"
        htmlType="button"
        style={{ marginRight: 10 }}
      >
        {I18N.get('suggestion.form.button.saveDraft')}
      </Button>
    )
    const cancelText = isEditMode
      ? I18N.get('suggestion.form.button.discardChanges')
      : I18N.get('suggestion.form.button.cancel')
    const cancelBtn = (
      <Button
        onClick={this.props.onCancel}
        className="cr-btn cr-btn-default"
        htmlType="button"
        style={{ marginRight: 10 }}
      >
        {cancelText}
      </Button>
    )
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
            {TAB_KEYS.map((item) => (
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
            {!this.hideContinue() && (
              <Button
                onClick={this.handleContinue}
                className="cr-btn cr-btn-black"
                htmlType="button"
              >
                {I18N.get('suggestion.form.button.continue')}
              </Button>
            )}
          </Row>

          <Row gutter={8} type="flex" justify="center">
            {cancelBtn}
            {saveDraftBtn}
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

  onTabChange = (activeKey) => {
    this.setState({ activeKey })
  }
}

export default Form.create()(C)
