import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Input, Modal, Select, Button } from 'antd'
import config from '@/config'

const FormItem = Form.Item

export default Form.create()(
  class C extends BaseComponent {
    ord_render () {
      const {visible, onCancel, onCreate, handleRemoveCountry, form} = this.props
      const {getFieldDecorator} = form
      const formItemLayout = {
        labelCol: {span: 6},
        wrapperCol: {span: 18}
      }

      const footerModal = (
        <div>
          <Button onClick={onCreate} type="primary" className="ant-btn-ebp">Change organizer</Button>
          {/* Don't need on this pharse */}
          {/* <Button onClick={handleRemoveCountry}>Remove country</Button> */}
          <Button onClick={onCancel}>Cancel</Button>
        </div>
      )

      const users = this.props.users || []

      return (
        <Modal
          visible={visible}
          title="Change organizer"
          footer={footerModal}
          okText="Create"
          onCancel={onCancel}
          onOk={onCreate}
        >
          <Form>
            <FormItem
              {...formItemLayout}
              label="Country">
              {getFieldDecorator('geolocation', {})(
                <Select
                  disabled={true}
                >
                  {Object.keys(config.data.mappingCountryCodeToName).map((key, index) => {
                    return (
                      <Select.Option title={config.data.mappingCountryCodeToName[key]} key={index}
                        value={key}>
                        {config.data.mappingCountryCodeToName[key]}
                      </Select.Option>
                    )
                  })}
                </Select>
              )}
            </FormItem>
            <FormItem
              {...formItemLayout}
              label="Organizer">
              {getFieldDecorator('leader', {
                rules: [{required: true, message: 'This field is required'}]
              })(
                <Select
                  showSearch={true}
                  placeholder="Please select a member"
                  filterOption={(input, option) => option.props.children.toLowerCase().indexOf(input.toLowerCase()) >= 0}
                >
                  {users.map((leader, index) => {
                    return (<Select.Option key={index} value={leader._id}>{leader.username}</Select.Option>)
                  })}
                </Select>
              )}
            </FormItem>
          </Form>
        </Modal>
      )
    }
  },
)
