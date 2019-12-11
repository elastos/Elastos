import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {Form, Input, Button, Select} from 'antd'
import {SUBMISSION_TYPE} from '@/constant'
import I18N from '@/I18N'

const FormItem = Form.Item
const Option = Select.Option
const TextArea = Input.TextArea

class C extends BaseComponent {

  handleSubmit(e) {
    e.preventDefault()
    this.props.form.validateFields((err, values) => {
      if (!err) {
        console.log('Received values of form: ', values)
        this.props.createSubmission(values.submissionType,
          values.submissionTitle, values.submissionDescription)
      }
    })
  }

  getSubmissionFormProps() {

    const {getFieldDecorator} = this.props.form

    const submissionType_fn = getFieldDecorator('submissionType', {
      rules: [{required: true, message: I18N.get('developer.form.submission.type.required')}],
      initialValue: SUBMISSION_TYPE.BUG
    })
    const submissionType_el = (
      <Select name="type">
        <Option value={SUBMISSION_TYPE.BUG}>{I18N.get('developer.form.submission.type.option.bug')}</Option>
        <Option value={SUBMISSION_TYPE.SECURITY_ISSUE}>{I18N.get('developer.form.submission.type.option.security')}</Option>
        <Option value={SUBMISSION_TYPE.SUGGESTION}>{I18N.get('developer.form.submission.type.option.suggestion')}</Option>
        <Option value={SUBMISSION_TYPE.OTHER}>{I18N.get('developer.form.submission.type.option.other')}</Option>
      </Select>
    )

    const submissionTitle_fn = getFieldDecorator('submissionTitle', {
      rules: [{required: true, message: I18N.get('developer.form.submission.title.required')}],
      initialValue: ''
    })
    const submissionTitle_el = (
      <Input name="title" placeholder={I18N.get('developer.form.submission.placeholder.subject')}/>
    )

    const submissionDescription_fn = getFieldDecorator('submissionDescription', {
      rules: [{required: true, message: I18N.get('developer.form.submission.description.required')}],
      initialValue: ''
    })
    const submissionDescription_el = (
      <TextArea rows={4} type="text" name="description" placeholder={I18N.get('developer.form.submission.placeholder.description')}/>
    )

    return {
      submissionType: submissionType_fn(submissionType_el),
      submissionTitle: submissionTitle_fn(submissionTitle_el),
      submissionDescription: submissionDescription_fn(submissionDescription_el)
    }
  }

  ord_render() {
    const {getFieldDecorator} = this.props.form
    const p = this.getSubmissionFormProps()
    return (
      <Form onSubmit={this.handleSubmit.bind(this)} className="c_issueForm">
        <FormItem>
          {p.submissionType}
        </FormItem>
        <FormItem>
          {p.submissionTitle}
        </FormItem>
        <FormItem>
          {p.submissionDescription}
        </FormItem>
        <FormItem>
          <Button loading={this.props.loading} type="ebp" htmlType="submit" className="d_btn pull-right">
                        Submit
          </Button>
        </FormItem>
      </Form>
    )
  }
}

export default Form.create()(C)
