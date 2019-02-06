import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import _ from 'lodash'
import {
  Row,
  Col,
  Form,
  Input,
  Button,
  Icon,
  Modal,
} from 'antd'
import I18N from '@/I18N'
import ReactQuill from 'react-quill'
import { TOOLBAR_OPTIONS } from '@/config/constant'
import sanitizeHtml from 'sanitize-html'
import './style.scss'

const FormItem = Form.Item

// TOTO: add mention module
// https://github.com/afconsult/quill-mention

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      isTranslateModalOpen: false,
      showRules: false
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
      <Input size="large" />
    )

    const textarea_el = (
      <ReactQuill
        modules={{
          toolbar: TOOLBAR_OPTIONS,
        }}
      />
    )

    const link_el = (
    <Input size="large" />
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

  translate = () => {
    console.log('translate: ', this.props.form.getFieldsValue(['title', 'description']));
  }

  renderTranslationModal() {
    const { isTranslateModalOpen } = this.state
    if (!isTranslateModalOpen) return null
    const translation = 'Translating...'
    // TODO: translation
    // translation =
    return (
      <Modal
        className="translate-modal-container"
        visible={this.state.isTranslateModalOpen}
        onOk={this.showTranslate}
        onCancel={this.showCreateForm}
        footer={null}
        width="70%"
      >
        {translation}
      </Modal>
    )
  }

  showTranslate = () => {
    this.setState({
      showTranslate: !this.state.isTranslateModalOpen,
    })
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

    const formItemLayout = {
      labelCol: {
        xs: { span: 24 },
        sm: { span: 4 },
      },
      wrapperCol: {
        xs: { span: 24 },
        sm: { span: 20 },
      },
    }

    const formContent = (
      <div>
        <FormItem className="form-title">
          {p.title}
        </FormItem>
        <FormItem className="form-desc">
          {p.description}
        </FormItem>
        <FormItem label={I18N.get('from.TaskCreateForm.label.info')} {...formItemLayout} className="form-link">
          {p.link}
        </FormItem>
        <FormItem wrapperCol={{ xs: { span: 24, offset: 0 }, sm: { span: 12, offset: 8 } }} className="form-actions">
          <Button type="ebp" className="cr-btn-default" onClick={this.props.showCreateForm}>
            {I18N.get('suggestion.cancel')}
          </Button>
          <Button loading={this.props.loading} type="ebp" htmlType="submit" className="cr-btn-default cr-btn-primary">
            {I18N.get('suggestion.submit')}
          </Button>
        </FormItem>
      </div>
    )
    // TODO
    // const translationModal = this.renderTranslationModal()
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
        {/* {translationModal} */}
      </div>
    )
  }
}

export default Form.create()(C)
