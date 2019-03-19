import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import _ from 'lodash'
import { CopyToClipboard } from 'react-copy-to-clipboard'
import {
  Row,
  Col,
  Form,
  Input,
  Button,
  Icon,
  Modal,
  message,
  Spin,
} from 'antd'
import I18N from '@/I18N'
import ReactQuill from 'react-quill'
import { TOOLBAR_OPTIONS } from '@/config/constant'
import sanitizeHtml from 'sanitize-html'
import './style.scss'
import { TranslateButton, CopyButton, ModalBody, TranslationText } from './style'

const FormItem = Form.Item

// TOTO: add mention module
// https://github.com/afconsult/quill-mention

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      isTranslateModalOpen: false,
      showRules: false,
      translation: '',
      copied: false,
    }
  }
  //   componentDidMount() {
  //     // TOTO: get council members used for mentions
  //   }

  handleSubmit(e) {
    e.preventDefault()

    this.props.form.validateFields(async (err, values) => {
      if (!err) {
        if (_.isEmpty(values.description)) {
          this.props.form.setFields({
            description: {
              errors: [new Error(I18N.get('suggestion.create.error.descriptionRequired'))],
            },
          })

          return
        }
        if (_.isEmpty(values.description)) {
          this.props.form.setFields({
            title: {
              errors: [new Error(I18N.get('suggestion.create.error.titleRequired'))],
            },
          })

          return
        }

        const createParams = {
          title: values.title,
          desc: sanitizeHtml(values.description, {
            allowedTags: sanitizeHtml.defaults.allowedTags.concat(['u', 's']),
          }),
          link: values.link
        }

        try {
          await this.props.create(createParams)
          this.props.showCreateForm()
          this.props.refetch()
        } catch (error) {
          // console.log(error)
        }
      }
    })
  }

  getInputProps() {
    const { getFieldDecorator } = this.props.form

    const input_el = (
      <Input size="large" placeholder="Title" onSelect={this.onSelect} />
    )

    const textarea_el = (
      <ReactQuill
        placeholder="Description"
        modules={{
          toolbar: TOOLBAR_OPTIONS,
          autoLinks: true,
        }}
        onSelect={this.onSelect}
      />
    )

    const link_el = (
      <Input size="large" placeholder="Info Link" />
    )

    const title_fn = getFieldDecorator('title', {
      rules: [
        { required: true, message: I18N.get('suggestion.create.error.titleRequired') },
        { min: 4, message: I18N.get('suggestion.create.error.titleTooShort') },
      ],
      initialValue: '',
    })

    const description_fn = getFieldDecorator('description', {
      rules: [
        { required: true, message: I18N.get('suggestion.create.error.descriptionRequired') },
        { min: 20, message: I18N.get('suggestion.create.error.descriptionTooShort') },
      ],
      initialValue: ''
    })

    const link_fn = getFieldDecorator('link', {
      rules: [
        {type: 'url'}
      ]
    })

    return {
      title: title_fn(input_el),
      description: description_fn(textarea_el),
      link: link_fn(link_el)
    }
  }

  onSelect = () => {
    const selectedText = window.getSelection().toString()
    console.log(selectedText)
    this.setState({ selectedText })
  }

  translate = async () => {
    const { gTranslate } = this.props
    const { selectedText } = this.state
    const formValues = this.props.form.getFieldsValue(['title', 'description'])
    console.log('translate: ', formValues);
    this.setState({ isTranslateModalOpen: true, translation: <Spin /> })
    const res = await gTranslate({ text: selectedText })
    // const res = await gTranslate({ text: formValues.title + formValues.description })
    this.setState({ translation: res.translation })
    console.log('res is: ', res.translation)
  }

  renderTranslationModal() {
    const { isTranslateModalOpen, translation } = this.state
    const copyBtn = <CopyButton>{I18N.get('suggestion.copy')}</CopyButton>
    return (
      <Modal
        className="translate-modal-container"
        visible={isTranslateModalOpen}
        onOk={this.showTranslate}
        onCancel={this.showTranslate}
        footer={null}
        width="70%"
        closable
        centered
        style={{ minWidth: 400 }}
      >
        <ModalBody>
          <TranslationText>{translation}</TranslationText>
          <Row type="flex" justify="space-between">
            <span>{I18N.get('suggestion.translatedByGoogle')}</span>
            <CopyToClipboard text={this.state.translation} onCopy={this.onCopy}>
              {copyBtn}
            </CopyToClipboard>
          </Row>
        </ModalBody>
      </Modal>
    )
  }

  showTranslate = () => {
    const { isTranslateModalOpen } = this.state
    this.setState({
      isTranslateModalOpen: !isTranslateModalOpen,
    })
  }

  onCopy = () => {
    this.setState({ copied: true })
    message.success(I18N.get('suggestion.copied'))
  }

  renderHeader() {
    return (
      <Row>
        <Col span={18}>
          <h2 className="title komu-a">
            {this.props.header || I18N.get('suggestion.add').toUpperCase()}
          </h2>
        </Col>
        <Col span={6}>
          <h5 className="alignRight">
            <a onClick={() => {this.setState({showRules: !this.state.showRules})}}>
              {I18N.get('suggestion.rules.rulesAndGuidelines')} <Icon type="question-circle"/>
            </a>
          </h5>
        </Col>
      </Row>
    )
  }

  renderRules() {
    return (
      <div>
        <h4>
          {I18N.get('suggestion.rules.guarantee')}
        </h4>

        <p>
          {I18N.get('suggestion.rules.response')}
        </p>

        <h4>
          {I18N.get('suggestion.rules.guidelines')}
        </h4>

        <ol>
          <li>{I18N.get('suggestion.rules.guidelines.1')}</li>
          <li>{I18N.get('suggestion.rules.guidelines.2')}</li>
          <li>{I18N.get('suggestion.rules.guidelines.3')}</li>
        </ol>

        <h4>
          {I18N.get('suggestion.rules')}
        </h4>

        <ol>
          <li>{I18N.get('suggestion.rules.1')}</li>
          <li>{I18N.get('suggestion.rules.2')}</li>
          <li>{I18N.get('suggestion.rules.3')}</li>
        </ol>

        <p>
          {I18N.get('suggestion.rules.infoRequest')}
        </p>

        <Button class="pull-right" onClick={() => {this.setState({showRules: false})}}>{I18N.get('suggestion.back')}</Button>
        <div class="clearfix">
          <br/>
        </div>
      </div>
    )
  }

  ord_render() {
    const headerNode = this.renderHeader()
    const rulesNode = this.renderRules()
    const p = this.getInputProps()

    const formContent = (
      <div>
        <FormItem className="form-title">
          {p.title}
        </FormItem>
        <FormItem className="form-desc">
          {p.description}
        </FormItem>
        <FormItem className="form-link">
          {p.link}
        </FormItem>
        <FormItem className="form-link">
          <TranslateButton onClick={this.translate}>{I18N.get('suggestion.translate')}</TranslateButton>
        </FormItem>
        <Row type="flex" justify="center">
          <Col xs={24} sm={12} md={6}>
            <Button type="ebp" className="cr-btn cr-btn-default" onClick={this.props.showCreateForm}>
              {I18N.get('suggestion.cancel')}
            </Button>
          </Col>
          <Col xs={24} sm={12} md={6}>
            <Button loading={this.props.loading} type="ebp" htmlType="submit" className="cr-btn cr-btn-primary">
              {I18N.get('suggestion.submit')}
            </Button>
          </Col>
        </Row>
      </div>
    )
    // TODO
    const translationModal = this.renderTranslationModal()
    return (
      <div className="c_SuggestionForm">
        {headerNode}
        {this.state.showRules ?
          rulesNode :
          <Form onSubmit={this.handleSubmit.bind(this)} className="d_SuggestionForm">
            {formContent}
          </Form>
        }
        {/* <div onClick={this.showTranslate}>{I18N.get('suggestion.translate')}</div> */}
        {translationModal}
      </div>
    )
  }
}

export default Form.create()(C)
