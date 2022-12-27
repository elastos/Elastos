import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Form, Modal, Select, Input, Button, Cascader } from 'antd'
import _ from 'lodash'
import config from '@/config'
import CommunityService from '@/service/CommunityService'

const FormItem = Form.Item

export default Form.create()(
  class C extends BaseComponent {
        state = {
          communityTrees: []
        }

        componentDidMount () {
          this.getCommunityTrees()
        }

        getCommunityTrees () {
          this.getAllCommunities().then((communityTrees) => {
            this.setState({
              communityTrees
            })
          })
        }

        async getAllCommunities() {
          const communityService = new CommunityService()

          return new Promise((resolve, reject) => {
            communityService.getAll().then((data) => {
              const cascaderItems = data.map((item) => {
                return {
                  value: item._id,
                  label: item.name,
                  parentId: item.parentCommunityId,
                }
              })

              const rootCascaderItems = _.filter(cascaderItems, {
                parentId: null
              })

              rootCascaderItems.forEach((rootCascaderItem) => {
                const children = _.filter(cascaderItems, {
                  parentId: rootCascaderItem.value
                })

                if (children && children.length) {
                  rootCascaderItem.children = children
                }
              })

              resolve(rootCascaderItems)
            }).catch((err) => {
              reject(err)
            })
          })
        }

        ord_render () {
          const {visible, onCancel, onCreate, form} = this.props
          const {getFieldDecorator} = form
          const formItemLayout = {
            labelCol: {span: 6},
            wrapperCol: {span: 18}
          }

          const footerModal = (
            <div>
              <Button onClick={onCreate} className="ant-btn-ebp" type="primary">Add community</Button>
              <Button onClick={onCancel}>Cancel</Button>
            </div>
          )

          const community_fn = getFieldDecorator('community', {
            initialValue: [],
            rules: [{required: true, message: 'This field is required'}]
          })
          const community_el = (
            <Cascader options={this.state.communityTrees} placeholder="" />
          )

          const state_fn = getFieldDecorator('state', {
            initialValue: '',
            rules: [{required: true, message: 'This field is required'}]
          })
          const state_el = (
            <Input/>
          )

          const city_fn = getFieldDecorator('city', {
            initialValue: '',
            rules: [{required: true, message: 'This field is required'}]
          })
          const city_el = (
            <Input/>
          )

          return (
            <Modal
              visible={visible}
              title="Add community"
              footer={footerModal}
              okText="Create"
              onCancel={onCancel}
              onOk={onCreate}
            >
              <Form>
                <FormItem {...formItemLayout} label="Community">
                  {community_fn(community_el)}
                </FormItem>
                <FormItem {...formItemLayout} label="State">
                  {state_fn(state_el)}
                </FormItem>
                <FormItem {...formItemLayout} label="City">
                  {city_fn(city_el)}
                </FormItem>
              </Form>
            </Modal>
          )
        }
  },
)
