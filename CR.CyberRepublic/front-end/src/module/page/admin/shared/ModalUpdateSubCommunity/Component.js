import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Input, Modal, Select, Button } from 'antd'
import {COMMUNITY_TYPE} from '@/constant'

import config from '@/config'

const FormItem = Form.Item

export default Form.create()(
  class C extends BaseComponent {
    ord_render () {
      const {visible, onCancel, onCreate, onDelete, form, communityType} = this.props
      const {getFieldDecorator} = form
      const formItemLayout = {
        labelCol: {span: 6},
        wrapperCol: {span: 18}
      }

      let contextTitle
      switch (communityType) {
        case COMMUNITY_TYPE.STATE:
          contextTitle = 'Update states / provinces'
          break
        case COMMUNITY_TYPE.CITY:
          contextTitle = 'Update city'
          break
        case COMMUNITY_TYPE.REGION:
          contextTitle = 'Update region'
          break
        default:
          contextTitle = 'Update school'
          break
      }

      const footerModal = (
        <div>
          <Button onClick={onCreate} type="primary" className="ant-btn-ebp">{contextTitle}</Button>
          <Button onClick={onDelete} type="danger">Delete</Button>
          <Button onClick={onCancel}>Cancel</Button>
        </div>
      )

      const users = this.props.users || []

      return (
        <Modal
          visible={visible}
          title={contextTitle}
          footer={footerModal}
          okText="Update"
          onCancel={onCancel}
          onOk={onCreate}
        >
          <Form>
            <FormItem
              {...formItemLayout}
              label="Country">
              {getFieldDecorator('country')(
                <Select
                  disabled={true}
                  showSearch={true}
                  placeholder="Please select a country"
                  filterOption={(input, option) => option.props.children.toLowerCase().indexOf(input.toLowerCase()) >= 0}
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
              label="Name">
              {getFieldDecorator('name', {
                rules: [{required: true, message: 'This field is required'}]
              })(
                <Input />
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
