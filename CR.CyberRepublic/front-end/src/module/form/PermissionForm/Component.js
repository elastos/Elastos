import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import _ from 'lodash'
import {
  Row,
  Col,
  Form,
  Input,
  Button,
} from 'antd'
import I18N from '@/I18N'

import { Container, StyledFormDesc } from './style'

const FormItem = Form.Item

class C extends BaseComponent {
  handleSubmit(e) {
    e.preventDefault()

    const {form, onFormSubmit, data} = this.props

    form.validateFields(async (err, values) => {
      if (!err) {
        const param = {
          name: values.name,
          resourceType: values.resourceType,
          desc: values.desc,
          httpMethod: values.httpMethod,
          url: values.url,
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

    const name_el = <Input size="large" />
    const resourceType_el = <Input size="large" />
    const desc_el = <Input size="large" />
    const httpMethod_el = <Input size="large" />
    const url_el = <Input size="large" />

    const name_fn = getFieldDecorator('name', {
      initialValue: _.get(data, 'name', ''),
    })

    const resourceType_fn = getFieldDecorator('resourceType', {
      initialValue: _.get(data, 'resourceType', ''),
    })

    const desc_fn = getFieldDecorator('desc', {
      initialValue: _.get(data, 'desc', ''),
    })

    const httpMethod_fn = getFieldDecorator('httpMethod', {
      initialValue: _.get(data, 'httpMethod', ''),
    })

    const url_fn = getFieldDecorator('url', {
      initialValue: _.get(data, 'url', ''),
    })

    return {
      name: name_fn(name_el),
      resourceType: resourceType_fn(resourceType_el),
      desc: desc_fn(desc_el),
      httpMethod: httpMethod_fn(httpMethod_el),
      url: url_fn(url_el),
    }
  }

  renderHeader() {
    const header = 'Create Permission'
    return (
      <h2 className="name komu-a">
        {header}
      </h2>
    )
  }

  ord_render() {
    const headerNode = this.renderHeader()
    const p = this.getInputProps()

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
        <FormItem label="Name" {...formItemLayout}>
          {p.name}
        </FormItem>
        <FormItem label="ResourceType" {...formItemLayout}>
          {p.resourceType}
        </FormItem>
        <FormItem label="Desc" {...formItemLayout}>
          {p.desc}
        </FormItem>
        <FormItem label="HttpMethod" {...formItemLayout}>
          {p.httpMethod}
        </FormItem>
        <FormItem label="Url" {...formItemLayout}>
          {p.url}
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
      <Container>
        {headerNode}
        <Form onSubmit={this.handleSubmit.bind(this)}>
          {formContent}
        </Form>
      </Container>
    )
  }
}

export default Form.create()(C)
