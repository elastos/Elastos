import React from 'react'
import PropTypes from 'prop-types'
import styled from 'styled-components'
import { Form, Input, Button } from 'antd'
import BaseComponent from '@/model/BaseComponent'
import I18N from '@/I18N'
import CodeMirrorEditor from '@/module/common/CodeMirrorEditor'

const FormItem = Form.Item

class TeamInfoForm extends BaseComponent {
  handleSubmit = e => {
    e.stopPropagation() // prevent event bubbling
    e.preventDefault()
    const { form, onSubmit } = this.props
    form.validateFields((err, values) => {
      if (!err) {
        onSubmit(values)
      }
    })
  }

  ord_render() {
    const { getFieldDecorator } = this.props.form
    const { item } = this.props
    return (
      <Wrapper>
        <Title>{I18N.get('suggestion.plan.createTeamInfo')}</Title>
        <Form onSubmit={this.handleSubmit}>
          <Label>
            <span>*</span>
            {I18N.get('suggestion.plan.teamMember')}
          </Label>
          <FormItem>
            {getFieldDecorator('member', {
              rules: [
                {
                  required: true,
                  message: I18N.get('suggestion.form.error.required')
                }
              ],
              initialValue: item && item.member ? item.member : ''
            })(<Input />)}
          </FormItem>

          <Label>
            <span>*</span>
            {I18N.get('suggestion.plan.role')}
          </Label>
          <FormItem>
            {getFieldDecorator('role', {
              rules: [
                {
                  required: true,
                  message: I18N.get('suggestion.form.error.required')
                }
              ],
              initialValue: item && item.role ? item.role : ''
            })(<Input />)}
          </FormItem>

          <Label gutter={-8}>
            <span>*</span>
            {I18N.get('suggestion.plan.responsibility')}
          </Label>
          <FormItem>
            {getFieldDecorator('responsibility', {
              rules: [
                {
                  required: true,
                  message: I18N.get('suggestion.form.error.required')
                }
              ],
              initialValue:
                item && item.responsibility ? item.responsibility : ''
            })(
              <CodeMirrorEditor
                content={item && item.responsibility ? item.responsibility : ''}
                name="responsibility"
                autofocus={false}
              />
            )}
          </FormItem>

          <Label gutter={-8}>{I18N.get('suggestion.plan.moreInfo')}</Label>
          <FormItem>
            {getFieldDecorator('info', {
              initialValue: item && item.info ? item.info : ''
            })(
              <CodeMirrorEditor
                content={item && item.info ? item.info : ''}
                name="info"
                autofocus={false}
              />
            )}
          </FormItem>

          <Actions>
            <Button
              className="cr-btn cr-btn-default"
              onClick={() => {
                this.props.onCancel()
              }}
            >
              {I18N.get('suggestion.cancel')}
            </Button>
            <Button className="cr-btn cr-btn-primary" htmlType="submit">
              {item
                ? I18N.get('suggestion.form.button.update')
                : I18N.get('suggestion.form.button.create')}
            </Button>
          </Actions>
        </Form>
      </Wrapper>
    )
  }
}

TeamInfoForm.propTypes = {
  onSubmit: PropTypes.func.isRequired,
  onCancel: PropTypes.func.isRequired,
  item: PropTypes.object
}

export default Form.create()(TeamInfoForm)

const Wrapper = styled.div`
  max-width: 650px;
  margin: 0 auto;
`
const Title = styled.div`
  font-size: 30px;
  line-height: 42px;
  color: #000000;
  text-align: center;
  margin-bottom: 42px;
`
const Label = styled.div`
  font-size: 17px;
  color: #000;
  display: block;
  margin-bottom: ${props => (props.gutter ? props.gutter : 10)}px;
  > span {
    color: #ff0000;
  }
`
const Actions = styled.div`
  display: flex;
  justify-content: center;
  > button {
    margin: 0 8px;
  }
`
