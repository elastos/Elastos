import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import styled from 'styled-components'
import _ from 'lodash'
import { Row, Col, Form, Input, Button } from 'antd'
import I18N from '@/I18N'
import CodeMirrorEditor from '@/module/common/CodeMirrorEditor'

import { Container } from './style'

const FormItem = Form.Item

class C extends BaseComponent {
  handleSubmit(e) {
    e.preventDefault()

    const { form, onFormSubmit, data } = this.props

    form.validateFields(async (err, values) => {
      if (!err) {
        const param = {
          title: values.title,
          resourceType: values.resourceType,
          desc: values.desc,
          httpMethod: values.httpMethod,
          url: values.url
        }
        if (_.get(data, '_id')) {
          param.id = _.get(data, '_id')
        }
        onFormSubmit(param)
      }
    })
  }

  getInputProps() {
    const { getFieldDecorator } = this.props.form
    const { data } = this.props

    const title = <Input size="large" />
    const title_fn = getFieldDecorator('title', {
      initialValue: _.get(data, 'title', '')
    })

    const desc = _.get(data, 'desc', '')
    const desc_fn = getFieldDecorator('desc', {
      initialValue: desc
    })
    const desc_el = <CodeMirrorEditor content={desc} name="desc" />

    return {
      title: title_fn(title),
      desc: desc_fn(desc_el)
    }
  }

  renderHeader() {
    const header = this.props.header || I18N.get('release.form.add')
    return <h2 className="title komu-a">{header}</h2>
  }

  ord_render() {
    const headerNode = this.renderHeader()
    const p = this.getInputProps()

    const formItemLayout = {
      labelCol: {
        span: 24
      },
      wrapperCol: {
        span: 24
      },
      colon: false
    }
    const formContent = (
      <div>
        <FormItem label="Name" {...formItemLayout}>
          {p.title}
        </FormItem>
        <StyledFormItem>
          <FormItem label="Desc" {...formItemLayout}>
            {p.desc}
          </FormItem>
        </StyledFormItem>

        <Row type="flex" justify="center">
          <Col xs={24} sm={12} md={6}>
            <Button
              type="ebp"
              className="cr-btn cr-btn-default"
              onClick={this.props.onFormCancel}
            >
              {I18N.get('suggestion.cancel')}
            </Button>
          </Col>
          <Col xs={24} sm={12} md={6}>
            <Button
              loading={this.props.loading}
              type="ebp"
              htmlType="submit"
              className="cr-btn cr-btn-primary"
            >
              {I18N.get('suggestion.submit')}
            </Button>
          </Col>
        </Row>
      </div>
    )

    return (
      <Container>
        {headerNode}
        <Form onSubmit={this.handleSubmit.bind(this)}>{formContent}</Form>
      </Container>
    )
  }
}

export default Form.create()(C)

const StyledFormItem = styled.div`
  .ant-col-24.ant-form-item-label {
    padding: 0;
    margin-bottom: -12px;
  }
`
