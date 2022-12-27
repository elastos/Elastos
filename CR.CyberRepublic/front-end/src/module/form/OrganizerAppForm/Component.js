import React from 'react'
import BaseComponent from '@/model/BaseComponent'
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

import {upload_file} from '@/util'
import I18N from '@/I18N'
import { getSafeUrl } from '@/util/url'

import './style.scss'

const FormItem = Form.Item
const TextArea = Input.TextArea

/**
 * This is form for members to apply to be an organizer
 *
 * TODO: this will later allow them to apply to be a vote candidate
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
      attachment_type: '',

      community: null
    }
  }

  async componentDidMount() {

    if (this.props.location && this.props.location.search) {
      const qry = this.props.location.search.match(/[\\?&]communityId=(\w+)/)
      if (qry.length > 1) {
        const communityId = qry[1]
        const community = await this.props.getCommunityDetail(communityId)
        this.setState({community})
      }
    }
  }

  getInputProps () {

    const {getFieldDecorator} = this.props.form

    // name
    const fullLegalName_fn = getFieldDecorator('fullLegalName', {
      rules: [
        {required: true, message: I18N.get('from.OrganizerAppForm.fullLegalName.required')},
        {min: 6, message: I18N.get('from.OrganizerAppForm.fullLegalName.min')}
      ]
    })
    const fullLegalName_el = (
      <Input size="large"/>
    )

    // occupation
    const occupation_fn = getFieldDecorator('occupation', {
      rules: [
        {required: true, message: I18N.get('from.OrganizerAppForm.occupation.required')},
        {min: 4, message: I18N.get('from.OrganizerAppForm.occupation.min')}
      ]
    })
    const occupation_el = (
      <Input size="large"/>
    )

    // education
    const education_fn = getFieldDecorator('education', {
      rules: [
        {required: true, message: I18N.get('from.OrganizerAppForm.education.required')},
        {min: 8, message: I18N.get('from.OrganizerAppForm.education.min')}
      ]
    })
    const education_el = (
      <Input size="large"/>
    )

    // audienceInfo
    const audienceInfo_fn = getFieldDecorator('audienceInfo', {
      rules: [
        {required: true, message: I18N.get('from.OrganizerAppForm.field.required')},
        {max: 256, message: I18N.get('from.OrganizerAppForm.field.max')}
      ]
    })
    const audienceInfo_el = (
      <TextArea rows={2} name="audienceInfo" />
    )

    // publicSpeakingExp
    const publicSpeakingExp_fn = getFieldDecorator('publicSpeakingExp', {
      rules: [
        {required: true, message: I18N.get('from.OrganizerAppForm.field.required')},
        {max: 1024, message: I18N.get('from.OrganizerAppForm.field.max')}
      ]
    })
    const publicSpeakingExp_el = (
      <TextArea rows={2} name="publicSpeakingExp" />
    )

    // eventOrganizingExp
    const eventOrganizingExp_fn = getFieldDecorator('eventOrganizingExp', {
      rules: [
        {required: true, message: I18N.get('from.OrganizerAppForm.field.required')},
        {max: 1024, message: I18N.get('from.OrganizerAppForm.field.max')}
      ]
    })
    const eventOrganizingExp_el = (
      <TextArea rows={2} name="eventOrganizingExp" />
    )

    // previousExp
    const previousExp_fn = getFieldDecorator('previousExp', {
      rules: [
        {required: true, message: I18N.get('from.OrganizerAppForm.field.required')},
        {max: 1024, message: I18N.get('from.OrganizerAppForm.field.max')}
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
        {max: 1024, message: I18N.get('from.OrganizerAppForm.field.max')}
      ]
    })
    const devBackground_el = (
      <TextArea rows={2} name="previousExp" />
    )

    // description
    const description_fn = getFieldDecorator('description', {
      rules: [
        {required: true, message: I18N.get('from.OrganizerAppForm.field.required')},
        {max: 512, message: I18N.get('from.OrganizerAppForm.field.max')}
      ]
    })
    const description_el = (
      <TextArea rows={1} name="description" />
    )

    // reason
    const reason_fn = getFieldDecorator('reason', {
      rules: [
        {required: true, message: I18N.get('from.OrganizerAppForm.field.required')},
        {max: 256, message: I18N.get('from.OrganizerAppForm.field.max')}
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
              {I18N.get('from.OrganizerAppForm.click.upload')}
            </Button>
          )
        }
      </Upload>
    )


    return {
      fullLegalName: fullLegalName_fn(fullLegalName_el),
      occupation: occupation_fn(occupation_el),
      education: education_fn(education_el),

      audienceInfo: audienceInfo_fn(audienceInfo_el),
      publicSpeakingExp: publicSpeakingExp_fn(publicSpeakingExp_el),
      eventOrganizingExp: eventOrganizingExp_fn(eventOrganizingExp_el),
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

    return (!this.props.is_login ? (
      <div>
        <br/>
        {I18N.get('from.OrganizerAppForm.mustlogged')}
      </div>
    ) : (
      <div className="c_organizerAppFormContainer">

        <Form onSubmit={this.handleSubmit.bind(this)} className="d_taskCreateForm">
          <div>
            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                <h4>
                  <span style={{fontWeight: 200}}>{I18N.get('from.OrganizerAppForm.community.apply')}</span>
                  {' '}
&nbsp;
                  {this.state.community && this.state.community.name}
                </h4>
                <br/>
              </Col>
            </Row>
            <FormItem label={I18N.get('from.OrganizerAppForm.fullLegalName')} {...formItemLayout}>
              {p.fullLegalName}
            </FormItem>
            <FormItem label={I18N.get('from.OrganizerAppForm.Occupation')} {...formItemLayout}>
              {p.occupation}
            </FormItem>
            <FormItem label={I18N.get('from.OrganizerAppForm.Education')} {...formItemLayout}>
              {p.education}
            </FormItem>
            <Divider />
            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.OrganizerAppForm.language')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.audienceInfo}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.OrganizerAppForm.speaking')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.publicSpeakingExp}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.OrganizerAppForm.organizer')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.eventOrganizingExp}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.OrganizerAppForm.contributions')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.previousExp}
            </FormItem>

            <FormItem label={I18N.get('from.OrganizerAppForm.areyoudeveloper')} {...formItemLayout}>
              {p.isDeveloper}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.OrganizerAppForm.notdeveloper')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.devBackground}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.OrganizerAppForm.describeElastos')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.description}
            </FormItem>

            <Row>
              <Col xs={{span: 24}} md={{offset: 8, span: 12}}>
                {I18N.get('from.OrganizerAppForm.inspired')}
              </Col>
            </Row>
            <FormItem {...formItemNoLabelLayout}>
              {p.reason}
            </FormItem>

            <Divider>{I18N.get('from.OrganizerAppForm.divider.submitvideo')}</Divider>

            <FormItem label={I18N.get('from.OrganizerAppForm.attachment')} {...formItemLayout}>
              {p.attachment}
            </FormItem>

            <FormItem wrapperCol={{xs: {span: 24, offset: 0}, sm: {span: 12, offset: 8}}}>
              <Button loading={this.props.loading} type="ebp" htmlType="submit" className="d_btn">
                {I18N.get('from.OrganizerAppForm.submit')}
              </Button>
            </FormItem>
          </div>
        </Form>
      </div>
    ))
  }

}
export default Form.create()(C)
