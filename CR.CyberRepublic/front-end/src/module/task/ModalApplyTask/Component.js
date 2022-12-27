import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Modal, Select, Button, Input } from 'antd'
import config from '@/config'

const TextArea = Input.TextArea

const FormItem = Form.Item

export default Form.create()(
  class C extends BaseComponent {

    ord_render () {
      const {onCancel, onCreate, form, visible} = this.props
      const {getFieldDecorator} = form
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

      const footerModal = (
        <div>
          <Button onClick={onCreate} type="primary" className="ant-btn-ebp">Apply</Button>
          <Button onClick={onCancel}>Cancel</Button>
        </div>
      )

      const selectOptions = [{value: 'self', text: 'Apply as Self Only'}]
      if (this.props.teamsOwned.length) {
        for (const team of this.props.teamsOwned) {
          selectOptions.push({
            value: team._id,
            text: team.name
          })
        }
      }

      const applyMsg_fn = getFieldDecorator('applyMsg')
      const applyMsg_el = (
        <TextArea rows={4} name="applyMsg" />
      )

      return (
        <Modal
          visible={visible}
          title="Apply for Task"
          footer={footerModal}
          okText="Create"
          onCancel={onCancel}
          onOk={onCreate}
        >
          <p>
                        You can either apply as yourself only or if you are a leader
                        of any teams, you can apply for your team.
          </p>
          <Form>
            <FormItem label="Team or Solo" {...formItemLayout}>
              {getFieldDecorator('applyId', {
                initialValue: 'self',
                rules: [{required: true, message: 'This field is required'}]
              })(
                <Select
                  showSearch={true}
                  filterOption={(input, option) => option.props.children.toLowerCase().indexOf(input.toLowerCase()) >= 0}
                >
                  {selectOptions.map((o) => {
                    return <Select.Option key={o.value} value={o.value}>{o.text}</Select.Option>
                  })}
                </Select>
              )}
            </FormItem>
            <FormItem label="Application Message" {...formItemLayout}>
              {applyMsg_fn(applyMsg_el)}
            </FormItem>
          </Form>
        </Modal>
      )
    }
  },
)
