import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Modal, Select, Button } from 'antd'
import config from '@/config'
import _ from 'lodash'

const FormItem = Form.Item

export default Form.create()(
  class C extends BaseComponent {
    ord_render () {
      const {visible, onCancel, onCreate, form} = this.props
      const {getFieldDecorator} = form
      const formItemLayout = {
        labelCol: {span: 6},
        wrapperCol: {span: 18}
      }

      const footerModal = (
        <div>
          <Button onClick={onCreate} className="ant-btn-ebp" type="primary">Add country</Button>
          <Button onClick={onCancel}>Cancel</Button>
        </div>
      )

      const listCountriesEl = _.entries(config.data.mappingCountryCodeToName).map(([key, val]) => {
        return (
          <Select.Option key={key} value={key}>
            {val}
          </Select.Option>
        )
      })

      const users = this.props.users || []

      return (
        <Modal
          visible={visible}
          title="Add country"
          footer={footerModal}
          okText="Create"
          onCancel={onCancel}
          onOk={onCreate}
        >
          <Form>
            <FormItem
              {...formItemLayout}
              label="Country">
              {getFieldDecorator('geolocation', {
                rules: [{required: true, message: 'This field is required'}]
              })(
                <Select
                  showSearch={true}
                  placeholder="Please select a country"
                  filterOption={(input, option) => option.props.children.toLowerCase().indexOf(input.toLowerCase()) >= 0}
                >
                  {listCountriesEl}
                </Select>
              )}
            </FormItem>
            <FormItem
              {...formItemLayout}
              label="Leader">
              {getFieldDecorator('leader', {
                rules: [{required: false, message: 'This field is required'}]
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
