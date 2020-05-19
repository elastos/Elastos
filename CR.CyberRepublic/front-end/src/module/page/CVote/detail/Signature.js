import React, { Component } from 'react'
import styled from 'styled-components'
import { Form, Button, Input } from 'antd'

const { TextArea } = Input
const FormItem = Form.Item

class Signature extends Component {
  constructor(props) {
    super(props)
    this.state = {}
  }

  handleSubmit = (e) => {
    e.stopPropagation() // prevent event bubbling
    e.preventDefault()
    const { form, onSubmit } = this.props
    form.validateFields((err, values) => {
      if (!err) {
        onSubmit(values)
      }
    })
  }

  render() {
    const { getFieldDecorator } = this.props.form
    const { item } = this.props
    return (
      <Wrapper>
        <Title>Apply Payment</Title>
        <Form onSubmit={this.handleSubmit}>
          <Label>
            <span>*</span>
            Reason
          </Label>
          <FormItem>
            {getFieldDecorator('reason', {
              rules: [
                {
                  required: true,
                  message: 'This field is required'
                }
              ]
            })(<TextArea rows={8} style={{ resize: 'none' }} />)}
          </FormItem>
          <Actions>
            <Button
              className="cr-btn cr-btn-default"
              onClick={() => {
                this.props.onCancel()
              }}
            >
              Cancel
            </Button>
            <Button className="cr-btn cr-btn-primary" htmlType="submit">
              Submit
            </Button>
          </Actions>
        </Form>
      </Wrapper>
    )
  }
}

export default Form.create()(Signature)

const Wrapper = styled.div`
  max-width: 500px;
  margin: 0 auto;
`
const Label = styled.div`
  font-size: 17px;
  color: #000;
  display: block;
  margin-bottom: 8px;
  > span {
    color: #ff0000;
  }
`
const Title = styled.div`
  font-size: 30px;
  line-height: 42px;
  color: #000000;
  text-align: center;
  margin-bottom: 42px;
`

const Actions = styled.div`
  display: flex;
  justify-content: center;
  > button {
    margin: 0 8px;
  }
`
