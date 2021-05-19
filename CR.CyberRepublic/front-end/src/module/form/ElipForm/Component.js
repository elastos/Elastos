import React, { Fragment } from 'react'
import _ from 'lodash'
import { Form, Input, Button, Row, Tabs, Typography, Modal, Col } from 'antd'
import BaseComponent from '@/model/BaseComponent'
import I18N from '@/I18N'
import CodeMirrorEditor from '@/module/common/CodeMirrorEditor'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import { ELIP_TYPE, ELIP_STATUS } from '@/constant'

import {
  Container,
  Title,
  TabPaneInner,
  Note,
  TabText,
  RadioCardPanel,
  RadioCardItem,
  RadioCardLabel,
  RadioCardSpan,
  Part,
  PartTitle,
  PartContent
} from './style'

const { Paragraph } = Typography
const FormItem = Form.Item
const { TabPane } = Tabs

const TABS = [
  {
    id: 'type',
    valueKey: 'elipType',
    rules: [
      {
        required: true,
        message: I18N.get('elip.form.error.required'),
      },
    ],
  },
  {
    id: 'abstract',
    valueKey: 'abstract',
    rules: [
      {
        required: true,
        message: I18N.get('elip.form.error.required'),
      },
    ],
  },
  {
    id: 'motivation',
    valueKey: 'abstract',
    rules: [
      {
        required: true,
        message: I18N.get('elip.form.error.required'),
      },
    ],
  },
  {
    id: 'specification',
    valueKey: 'specifications',
    rules: [
      {
        required: true,
        message: I18N.get('elip.form.error.required'),
      },
    ],
  },
  {
    id: 'rationale',
    valueKey: 'rationale',
    rules: [],
  },
  {
    id: 'backwardCompatibility',
    valueKey: 'backwardCompatibility',
    rules: [],
  },
  {
    id: 'referenceImplementation',
    valueKey: 'referenceImplementation',
    rules: [],
  },
  {
    id: 'copyright',
    valueKey: 'copyright',
    rules: [
      {
        required: true,
        message: I18N.get('elip.form.error.required'),
      },
    ],
  }
]
const TAB_KEYS = _.map(TABS, value => value.id)
const PREVIEW_EXCLUDE_KEYS = ['title', 'type']
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

const editors = {}

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      loading: false,
      activeKey: TAB_KEYS[0],
      errorKeys: {},
      isPreview: false
    }

    _.forEach(TABS, value => {
      editors[value.id] = null
    })
  }

  getActiveKey(key) {
    return !TAB_KEYS.includes(key) ? this.state.activeKey : key
  }

  handleSubmit = async (e, status) => {
    const { onSubmit, form } = this.props
    if (!status) {
      this.setState({ loading: true })
    }
    e.preventDefault()
    form.validateFields(async (err, values) => {
      if (err) {
        const data = {
          errorKeys: err,
          activeKey: this.getActiveKey(Object.keys(err)[0])
        }
        if (!status) {
          data.loading = false
        }
        this.setState(data)
        return
      }
      values.elipType = values.type
      values.specifications = values.specification
      delete values.specification
      delete values.type
      if (status) {
        values.status = status
      }
      await onSubmit(values)
      if (!status) {
        this.setState({ loading: false })
      }
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

  handlePreview = e => {
    const { isPreview } = this.state
    this.setState({ isPreview: !isPreview })
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

  getTypeRadioGroup(item) {
    const { data = {} } = this.props
    const { getFieldDecorator } = this.props.form
    return getFieldDecorator(item.id, {
      rules: item.rules,
      initialValue:
        data && data[item.valueKey]
          ? data[item.valueKey]
          : ELIP_TYPE.STANDARD_TRACK
    })(<RadioCard radioKey={item.id} />)
  }

  init(activeKey, editor) {
    editors[activeKey] = editor
  }

  getTextarea(item) {
    const { data = {} } = this.props
    const { getFieldDecorator } = this.props.form
    return getFieldDecorator(item.id, {
      rules: item.rules,
      initialValue: data[item.valueKey]
    })(
      <CodeMirrorEditor
        content={data[item.id]}
        callback={this.onTextareaChange}
        activeKey={this.state.activeKey}
        name={item.id}
        init={this.init}
      />
    )
  }

  renderTabText(item) {
    const hasError = _.has(this.state.errorKeys, item.id)
    const requiredFlag = _.isEmpty(item.rules) ? '' : '*'
    return (
      <TabText hasErr={hasError}>
        {I18N.get(`elip.fields.${item.id}`) + requiredFlag}
      </TabText>
    )
  }

  renderTabItem(item) {
    let formItem
    if (item.id === 'type') {
      formItem = this.getTypeRadioGroup(item)
    } else {
      formItem = this.getTextarea(item)
    }
    return (
      <TabPane tab={this.renderTabText(item)} key={item.id}>
        <TabPaneInner>
          <Note>{I18N.get(`elip.form.note.${item.id}`)}</Note>
          <FormItem>{formItem}</FormItem>
        </TabPaneInner>
      </TabPane>
    )
  }

  renderPreview() {
    const { form } = this.props
    const { isPreview } = this.state
    const fieldsValue = form.getFieldsValue()
    return (
      <Modal
        visible={isPreview}
        onOk={this.handlePreview}
        onCancel={this.handlePreview}
        cancelButtonProps={{ style: { display: 'none' } }}
      >
        {isPreview &&
          _.map(_.difference(TAB_KEYS, PREVIEW_EXCLUDE_KEYS), value => (
            <Part id={value} key={value}>
              <PartTitle>{I18N.get(`elip.fields.${value}`)}</PartTitle>
              <PartContent>
                <MarkdownPreview
                  content={fieldsValue[value] ? fieldsValue[value] : ''}
                />
              </PartContent>
            </Part>
          ))}
      </Modal>
    )
  }

  renderPersonalDraftButton() {
    const { data } = this.props
    if (!data || (data && data.status === ELIP_STATUS.PERSONAL_DRAFT)) {
      return (
        <Col>
          <Button
            onClick={e => {
              this.handleSubmit(e, ELIP_STATUS.PERSONAL_DRAFT)
            }}
            className="cr-btn cr-btn-primary"
            htmlType="button"
          >
            {I18N.get('elip.button.personalDraft')}
          </Button>
        </Col>
      )
    }
  }
  renderReviewButton() {
    const { data } = this.props
    const rs = [ELIP_STATUS.PERSONAL_DRAFT, ELIP_STATUS.REJECTED]
    if (!data || (data && rs.includes(data.status))) {
      return (
        <Col>
          <Button
            loading={this.state.loading}
            className="cr-btn cr-btn-primary"
            htmlType="submit"
          >
            {this.props.submitName || I18N.get('elip.button.submit')}
          </Button>
        </Col>
      )
    }
  }
  renderDraftButton() {
    const { data } = this.props
    if (data && data.status === ELIP_STATUS.DRAFT) {
      return (
        <Fragment>
          <Col>
            <Button
              onClick={e => this.handleSubmit(e, ELIP_STATUS.DRAFT)}
              className="cr-btn cr-btn-primary"
              htmlType="button"
            >
              {I18N.get('elip.button.saveChanges')}
            </Button>
          </Col>
          <Col>
            <Button
              onClick={e => {
                this.handleSubmit(e, ELIP_STATUS.FINAL_REVIEW)
              }}
              className="cr-btn cr-btn-primary"
              htmlType="button"
            >
              {I18N.get('elip.button.submittedAsProposal')}
            </Button>
          </Col>
        </Fragment>
      )
    }
  }

  ord_render() {
    const { form, data, onCancel } = this.props
    const { getFieldDecorator } = form
    const previewModal = this.renderPreview()
    return (
      <Container>
        <Form
          onSubmit={e => {
            this.handleSubmit(e, ELIP_STATUS.WAIT_FOR_REVIEW)
          }}
        >
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
            forceRender={true}
          >
            {_.map(TABS, value => this.renderTabItem(value))}
          </Tabs>
          <Row type="flex" justify="center" style={{ marginBottom: '30px' }}>
            <Col>
              <Button
                loading={this.state.loading}
                onClick={this.handleContinue}
                className="cr-btn cr-btn-black"
                htmlType="button"
              >
                {I18N.get('suggestion.form.button.continue')}
              </Button>
            </Col>
          </Row>
          <Row gutter={10} type="flex" justify="center">
            <Col>
              <Button onClick={onCancel} className="cr-btn cr-btn-default">
                {data
                  ? I18N.get('elip.button.back')
                  : I18N.get('elip.button.cancel')}
              </Button>
            </Col>
            <Col>
              <Button
                onClick={this.handlePreview}
                className="cr-btn cr-btn-default"
                htmlType="button"
              >
                {I18N.get('elip.button.preview')}
              </Button>
            </Col>
            {this.renderPersonalDraftButton()}
            {this.renderDraftButton()}
            {this.renderReviewButton()}
          </Row>
        </Form>
        {previewModal}
      </Container>
    )
  }

  onTabChange = activeKey => {
    this.setState({ activeKey })
    setTimeout(() => {
      editors[activeKey].focus()
    }, 20)
  }
}

class RadioCard extends BaseComponent {
  constructor(props) {
    super(props)
    this.state = {
      data: this.props.value
    }
    this.handleChange = this.handleChange.bind(this)
  }

  renderContent(id) {
    const contentStr = I18N.get(`elip.form.type.${id}`)
    const contentArr = _.split(contentStr, '<br />')
    const content = _.map(contentArr, (v, n) => {
      return <Paragraph key={n}>{v}</Paragraph>
    })
    return (
      <Typography>
        <RadioCardSpan style={{ fontWeight: 'bold', marginBottom: '1em' }}>
          {I18N.get(`elip.form.typeTitle.${id}`)}
        </RadioCardSpan>
        {content}
      </Typography>
    )
  }

  handleChange(evt) {
    this.setState({ data: evt.target.value })
    this.props.onChange(evt.target.value)
  }

  ord_render() {
    const { radioKey } = this.props
    return (
      <RadioCardPanel>
        <RadioCardItem className={`small-label ${this.state.data === ELIP_TYPE.STANDARD_TRACK ? 'radio-card-check' : ''}`}>
          <RadioCardLabel>
            <RadioCardSpan>
              <Input
                type="radio"
                name={radioKey}
                value={ELIP_TYPE.STANDARD_TRACK}
                style={{ display: 'none' }}
                onChange={this.handleChange}
              />
            </RadioCardSpan>
            <RadioCardSpan>
              <Paragraph>
                {this.renderContent(ELIP_TYPE.STANDARD_TRACK)}
              </Paragraph>
            </RadioCardSpan>
          </RadioCardLabel>
        </RadioCardItem>
        <RadioCardItem className={`small-label ${this.state.data === ELIP_TYPE.INFORMATIONAL ? 'radio-card-check' : ''}`}>
          <RadioCardLabel>
            <RadioCardSpan>
              <Input
                type="radio"
                name={radioKey}
                value={ELIP_TYPE.INFORMATIONAL}
                style={{ display: 'none' }}
                onChange={this.handleChange}
              />
            </RadioCardSpan>
            <RadioCardSpan>
              <Paragraph>
                {this.renderContent(ELIP_TYPE.INFORMATIONAL)}
              </Paragraph>
            </RadioCardSpan>
          </RadioCardLabel>
        </RadioCardItem>
        <RadioCardItem className={`large-label ${this.state.data === ELIP_TYPE.PROCESS ? 'radio-card-check' : ''}`}>
          <RadioCardLabel>
            <RadioCardSpan>
              <Input
                type="radio"
                name={radioKey}
                value={ELIP_TYPE.PROCESS}
                style={{ display: 'none' }}
                onChange={this.handleChange}
              />
            </RadioCardSpan>
            <RadioCardSpan>
              <Paragraph>{this.renderContent(ELIP_TYPE.PROCESS)}</Paragraph>
            </RadioCardSpan>
          </RadioCardLabel>
        </RadioCardItem>
      </RadioCardPanel>
    )
  }
}

export default Form.create()(C)
