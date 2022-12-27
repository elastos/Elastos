import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import I18N from '@/I18N'
import {
  Form,
  Icon,
  Input,
  InputNumber,
  Button,
  Checkbox,
  Select,
  message,
  Row,
  Col,
  Upload,
  Cascader,
  Divider

} from 'antd'
import { getSafeUrl } from '@/util/url'
import {upload_file} from '@/util'

const FormItem = Form.Item
const TextArea = Input.TextArea

/**
 * This is public form for the Evangelist Training
 */
class C extends BaseComponent {

  handleSubmit (e) {
    e.preventDefault()
    this.props.form.validateFields((err, values) => {
      if (!err) {
        this.props.submitForm(values, this.state)
      }
    })
  }

  constructor (props) {
    super(props)

    this.state = {
      attachment_url: null,
      attachment_loading: false,
      attachment_filename: '',
      attachment_type: ''
    }
  }

  getInputProps () {

    const {getFieldDecorator} = this.props.form

    // email
    const email_fn = getFieldDecorator('email', {
      rules: [
        {required: true, message: I18N.get('from.Training1Form.field.required')},
        {type: 'email', message: I18N.get('from.Training1Form.field.email.invalid')},
        {min: 6, message: I18N.get('from.Training1Form.field.min')}
      ]
    })
    const email_el = (
      <Input size="large"/>
    )

    // name
    const fullLegalName_fn = getFieldDecorator('fullLegalName', {
      rules: [
        {required: true, message: I18N.get('from.Training1Form.field.required')},
        {min: 6, message: I18N.get('from.Training1Form.field.min')}
      ]
    })
    const fullLegalName_el = (
      <Input size="large"/>
    )

    // occupation
    const occupation_fn = getFieldDecorator('occupation', {
      rules: [
        {required: true, message: I18N.get('from.Training1Form.field.required')},
        {min: 4, message: I18N.get('from.Training1Form.field.min')}
      ]
    })
    const occupation_el = (
      <Input size="large"/>
    )

    // education
    const education_fn = getFieldDecorator('education', {
      rules: [
        {required: true, message: I18N.get('from.Training1Form.field.required')},
        {min: 8, message: I18N.get('from.Training1Form.field.min')}
      ]
    })
    const education_el = (
      <Input size="large"/>
    )

    // audienceInfo
    const audienceInfo_fn = getFieldDecorator('audienceInfo', {
      rules: [
        {required: true, message: I18N.get('from.Training1Form.field.required')},
        {max: 256, message: I18N.get('from.Training1Form.field.max')}
      ]
    })
    const audienceInfo_el = (
      <TextArea rows={2} name="audienceInfo" />
    )

    // publicSpeakingExp
    const publicSpeakingExp_fn = getFieldDecorator('publicSpeakingExp', {
      rules: [
        {required: true, message: I18N.get('from.Training1Form.field.required')},
        {max: 1024, message: I18N.get('from.Training1Form.field.max')}
      ]
    })
    const publicSpeakingExp_el = (
      <TextArea rows={2} name="publicSpeakingExp" />
    )

    // previousExp
    const previousExp_fn = getFieldDecorator('previousExp', {
      rules: [
        {required: true, message: I18N.get('from.Training1Form.field.required')},
        {max: 1024, message: I18N.get('from.Training1Form.field.max')}
      ]
    })
    const previousExp_el = (
      <TextArea rows={1} name="previousExp" />
    )

    // developer
    const isDeveloper_fn = getFieldDecorator('isDeveloper')
    const isDeveloper_el = (
      <Checkbox/>
    )

    // devBackground
    const devBackground_fn = getFieldDecorator('devBackground', {
      rules: [
        {max: 1024, message: I18N.get('from.Training1Form.field.max')}
      ]
    })
    const devBackground_el = (
      <TextArea rows={2} name="previousExp" />
    )

    // description
    const description_fn = getFieldDecorator('description', {
      rules: [
        {required: true, message: I18N.get('from.Training1Form.field.required')},
        {max: 512, message: I18N.get('from.Training1Form.field.max')}
      ]
    })
    const description_el = (
      <TextArea rows={1} name="description" />
    )

    // reason
    const reason_fn = getFieldDecorator('reason', {
      rules: [
        {required: true, message: I18N.get('from.Training1Form.field.required')},
        {max: 256, message: I18N.get('from.Training1Form.field.max')}
      ]
    })
    const reason_el = (
      <TextArea rows={1} name="reason" />
    )

    // attachment
    const attachment_fn = getFieldDecorator('attachment', {
      rules: []
    })
    const p_attachment = {
      showUploadList: false,
      customRequest: (info) => {
        this.setState({
          attachment_loading: true
        })
        upload_file(info.file).then((d) => {
          const url = d.url
          this.setState({
            attachment_loading: false,
            attachment_url: url,
            attachment_type: d.type,
            attachment_filename: d.filename
          })
        })
      }
    }
    const attachment_el = (
      <Upload name="attachment" {...p_attachment}>
        {
          this.state.attachment_url ? (
            <a target="_blank" href={getSafeUrl(this.state.attachment_url)}>
              <Icon type="file"/>
              {' '}
&nbsp;
              {this.state.attachment_filename}
            </a>
          ) : (
            <Button loading={this.state.attachment_loading}>
              <Icon type="upload" />
              {' '}
              {I18N.get('from.Training1Form.button.upload')}
            </Button>
          )
        }
      </Upload>
    )


    return {
      email: email_fn(email_el),
      fullLegalName: fullLegalName_fn(fullLegalName_el),
      occupation: occupation_fn(occupation_el),
      education: education_fn(education_el),

      audienceInfo: audienceInfo_fn(audienceInfo_el),
      publicSpeakingExp: publicSpeakingExp_fn(publicSpeakingExp_el),
      previousExp: previousExp_fn(previousExp_el),

      isDeveloper: isDeveloper_fn(isDeveloper_el),

      devBackground: devBackground_fn(devBackground_el),

      description: description_fn(description_el),
      reason: reason_fn(reason_el),

      attachment: attachment_fn(attachment_el)
    }
  }

  ord_render () {
    const {getFieldDecorator} = this.props.form
    const p = this.getInputProps()

    const formItemLayout = {
      labelCol: {
        xs: {span: 24},
        sm: {span: 8},
      },
      wrapperCol: {
        xs: {span: 24},
        sm: {span: 12},
      },
    }

    const formItemNoLabelLayout = {
      wrapperCol: {
        xs: {span: 24},
        sm: {offset: 8, span: 12},
      },
    }


    // const existingTask = this.props.existingTask

    // TODO: terms of service checkbox\

    // TODO: react-motion animate slide left

    // TODO: description CKE Editor

    return (
      <div className="c_taskCreateFormContainer">

        <Form onSubmit={this.handleSubmit.bind(this)} className="d_training1Form">
          <div>
            <FormItem label={I18N.get('from.Training1Form.label.email')} {...formItemLayout}>
              {p.email}
            </FormItem>
            <FormItem label={I18N.get('from.Training1Form.label.fullllegal')} {...formItemLayout}>
              {p.fullLegalName}
            </FormItem>
            <FormItem label={I18N.get('from.Training1Form.label.occupation')} {...formItemLayout}>
              {p.occupation}
            </FormItem>
            <FormItem label={I18N.get('from.Training1Form.label.education')} {...formItemLayout}>
              {p.education}
            </FormItem>
            <Divider />
            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.Training1Form.text.nativelanguage')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.audienceInfo}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.Training1Form.text.describeyour')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.publicSpeakingExp}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.Training1Form.text.listanycurrent')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.previousExp}
            </FormItem>

            <FormItem label={I18N.get('from.Training1Form.text.adeveloper')} {...formItemLayout}>
              {p.isDeveloper}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.Training1Form.text.explain')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.devBackground}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.Training1Form.text.describe')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.description}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.Training1Form.text.tellfeword')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.reason}
            </FormItem>

            <Divider>{I18N.get('from.Training1Form.text.submitvideo')}</Divider>

            <FormItem label={I18N.get('from.Training1Form.label.attachment')} {...formItemLayout}>
              {p.attachment}
            </FormItem>

            <FormItem wrapperCol={{xs: {span: 24, offset: 0}, sm: {span: 12, offset: 8}}}>
              <Button loading={this.props.loading} type="ebp" htmlType="submit" className="d_btn">
                {I18N.get('from.Training1Form.button.submit')}
              </Button>
            </FormItem>
          </div>
        </Form>
      </div>
    )
  }

}
export default Form.create()(C)
