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
  DatePicker,
} from 'antd'
import moment from 'moment/moment'
import I18N from '@/I18N'
import ReactQuill from 'react-quill'
import {TOOLBAR_OPTIONS} from '@/config/constant'
import Translation from '@/module/common/Translation/Container'
import sanitizeHtml from 'sanitize-html'

import './style.scss'
import {StyledFormDesc} from './style'

const FormItem = Form.Item

// TOTO: add mention module
// https://github.com/afconsult/quill-mention

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      showRules: false,
    }
  }

  //   componentDidMount() {
  //     // TOTO: get council members used for mentions
  //   }

  handleSubmit(e) {
    e.preventDefault()

    const {form, onFormSubmit, data} = this.props

    form.validateFields(async (err, values) => {
      if (!err) {
        const param = {
          title: values.title,
          shortDesc: values.shortDesc,
          desc: sanitizeHtml(values.description, {
            allowedTags: sanitizeHtml.defaults.allowedTags.concat(['u', 's']),
          }),
          benefits: values.benefits,
        }
        if (!_.isEmpty(values.funding)) {
          param.funding = values.funding
        }
        if (!_.isEmpty(values.timeline)) {
          param.timeline = values.timeline
        }
        if (!_.isEmpty(values.link)) {
          param.link = _.map(_.split(values.link, ','), value => _.trim(value))
        }
        if (_.get(data, '_id')) {
          param.id = _.get(data, '_id')
        }
        onFormSubmit(param)
      }
    })
  }

  getInputProps() {
    const {getFieldDecorator} = this.props.form
    const {data} = this.props

    const input_el = <Input size="large"/>
    const shortDesc_el = <Input size="large"/>
    const textarea_el = (
      <ReactQuill
        modules={{
          toolbar: TOOLBAR_OPTIONS,
          autoLinks: true,
        }}
        style={{backgroundColor: 'white'}}
      />
    )
    const benefits_el = <Input.TextArea/>
    const funding_el = <Input size="large"/>
    const timeline_el = <DatePicker size="large" placeholder="" style={{width: '100%'}}/>
    const link_el = <Input size="large"/>

    const title_fn = getFieldDecorator('title', {
      rules: [
        {required: true, message: I18N.get('suggestion.create.error.required')},
        {min: 4, message: I18N.get('suggestion.create.error.tooShort')},
      ],
      initialValue: _.get(data, 'title', ''),
    })

    const shortDesc_fn = getFieldDecorator('shortDesc', {
      rules: [
        {required: true, message: I18N.get('suggestion.create.error.required')},
        {min: 20, message: I18N.get('suggestion.create.error.tooShort')},
        {max: 255, message: I18N.get('from.OrganizerAppForm.field.max')},
      ],
      initialValue: _.get(data, 'shortDesc', ''),
    })

    const description_fn = getFieldDecorator('description', {
      rules: [
        {required: true, message: I18N.get('suggestion.create.error.required')},
        {min: 20, message: I18N.get('suggestion.create.error.tooShort')},
      ],
      initialValue: _.get(data, 'desc', ''),
    })

    const benefits_fn = getFieldDecorator('benefits', {
      rules: [
        {required: true, message: I18N.get('suggestion.create.error.required')},
        {min: 20, message: I18N.get('suggestion.create.error.tooShort')},
      ],
      initialValue: _.get(data, 'benefits', ''),
    })

    const funding_fn = getFieldDecorator('funding', {
      rules: [
        {required: false},
      ],
      initialValue: _.get(data, 'funding', ''),
    })

    const timeline_fn = getFieldDecorator('timeline', {
      rules: [
        {required: false},
      ],
      initialValue: _.get(data, 'timeline') ? moment(_.get(data, 'timeline')) : undefined,
    })

    const link_fn = getFieldDecorator('link', {
      rules: [
        // { type: 'url' },
        { required: false },
      ],
      initialValue: _.join(_.get(data, 'link', ''), ','),
    })

    return {
      title: title_fn(input_el),
      shortDesc: shortDesc_fn(shortDesc_el),
      description: description_fn(textarea_el),
      benefits: benefits_fn(benefits_el),
      funding: funding_fn(funding_el),
      timeline: timeline_fn(timeline_el),
      link: link_fn(link_el),
    }
  }

  renderTranslationBtn() {
    const {title, description, benefits} = this.props.form.getFieldsValue(['title', 'description', 'benefits'])
    const text = `
      <h1>${title}</h1>
      <h4>${I18N.get('suggestion.form.fields.desc')}</h4>
      ${description}
      <h4>${I18N.get('suggestion.form.fields.benefits')}</h4>
      <p>${benefits}</p>
    `

    return (
      <div>
        <Translation text={text}/>
      </div>
    )
  }

  renderHeader() {
    let header = this.props.header || I18N.get('suggestion.add').toUpperCase()
    if (this.state.showRules) {
      header = I18N.get('suggestion.rules.rulesAndGuidelines')
    }
    return (
      <Row>
        <Col span={18}>
          <h2 className="title komu-a">
            {header}
          </h2>
        </Col>
        <Col span={6}>
          <h5 className="alignRight">
            <a onClick={() => {
              this.setState({showRules: !this.state.showRules})
            }}>
              {I18N.get('suggestion.rules.rulesAndGuidelines')}
              {' '}
              <Icon type="question-circle"/>
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

        <Button class="pull-right" onClick={() => {
          this.setState({showRules: false})
        }}>
          {I18N.get('suggestion.back')}
        </Button>
        <div className="clearfix">
          <br/>
        </div>
      </div>
    )
  }

  ord_render() {
    const headerNode = this.renderHeader()
    const rulesNode = this.renderRules()
    const p = this.getInputProps()
    const translationBtn = this.renderTranslationBtn()

    const formItemLayout = {
      labelCol: {
        span: 24,
      },
      wrapperCol: {
        span: 24,
      },
      colon: false,
    }
    const formContent = (
      <div>
        <FormItem className="form-item" label={I18N.get('suggestion.form.fields.subject')} {...formItemLayout}>
          {p.title}
        </FormItem>
        <FormItem className="form-item" label={I18N.get('suggestion.form.fields.shortDesc')} {...formItemLayout}>
          {p.shortDesc}
        </FormItem>
        <FormItem className="form-desc" label={I18N.get('suggestion.form.fields.desc')} {...formItemLayout}>
          {p.description}
        </FormItem>
        <FormItem className="form-item" label={I18N.get('suggestion.form.fields.benefits')} {...formItemLayout}>
          {p.benefits}
        </FormItem>
        <Row gutter={12}>
          <Col span={12}>
            <FormItem className="form-item" label={I18N.get('suggestion.form.fields.funding')} {...formItemLayout}>
              {p.funding}
            </FormItem>
          </Col>
          <Col span={12}>
            <FormItem className="form-item" label={I18N.get('suggestion.form.fields.timeline')} {...formItemLayout}>
              {p.timeline}
            </FormItem>
          </Col>
        </Row>
        <FormItem className="form-item" label={I18N.get('suggestion.form.fields.linksSplit')} {...formItemLayout}>
          {p.link}
        </FormItem>
        <FormItem className="form-item">
          {translationBtn}
        </FormItem>
        <Row type="flex" justify="center">
          <Col xs={24} sm={12} md={6}>
            <Button type="ebp" className="cr-btn cr-btn-default" onClick={this.props.onFormCancel}>
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

    return (
      <div className="c_SuggestionForm">
        {headerNode}
        {this.state.showRules ?
          rulesNode : (
            <Form onSubmit={this.handleSubmit.bind(this)} className="d_SuggestionForm">
              {formContent}
            </Form>
          )
        }
      </div>
    )
  }
}

export default Form.create()(C)
