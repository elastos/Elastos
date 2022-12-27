import React from 'react'
import _ from 'lodash'
import BaseComponent from '@/model/BaseComponent'
import {
  Form,
  Icon,
  Input,
  Button,
  Select,
  message,
  Row,
  Col,
  Upload,
} from 'antd'
import config from '@/config'
import { USER_AVATAR_DEFAULT } from '@/constant'
import I18N from '@/I18N'
import { upload_file } from '@/util'
import './style.scss'

const FormItem = Form.Item

class C extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      avatar: _.get(props, 'user.profile.avatar'),
    }
  }

  handleSubmit(e) {
    e.preventDefault()
    this.props.form.validateFields((err, values) => {
      if (!err) {
        this.props.updateUser(values, this.state).then(() => {
          this.props.refetch()
          this.props.switchEditMode()
          message.success(I18N.get('profile.thanksForCompleting'))
        })
      }
    })
  }

  getFormItemLayout() {
    return {
      colon: false,
      labelCol: {
        xs: { span: 12 },
        sm: { span: 8 },
      },
      wrapperCol: {
        xs: { span: 24 },
        sm: { span: 16 },
      },
    }
  }

  getInputProps() {
    const { getFieldDecorator } = this.props.form
    const { user } = this.props

    const role_fn = getFieldDecorator('role', {
      rules: [{ required: true, message: I18N.get('user.edit.form.label_role') }],
      initialValue: user.role,
    })
    const role_el = (
      <Select
        showSearch={true}
        filterOption={(input, option) => option.props.children.toLowerCase().indexOf(input.toLowerCase()) >= 0}
        placeholder="Role"
        // Fix select dropdowns in modals
        // https://github.com/vazco/uniforms/issues/228
        getPopupContainer={(x) => {
          while (x && x.tagName.toLowerCase() !== 'form') {
            x = x.parentElement
          }

          return x
        }}
      >
        {_.entries(config.data.mappingRoleToName).map(([key, val]) => (
          <Select.Option key={key} value={key}>
            {I18N.get(val)}
          </Select.Option>
        ))}
      </Select>
    )

    const firstName_fn = getFieldDecorator('firstName', {
      rules: [{ required: true, message: I18N.get('from.UserEditForm.firstName.required') }],
      initialValue: user.profile.firstName,
    })
    const firstName_el = (
      <Input />
    )

    const lastName_fn = getFieldDecorator('lastName', {
      rules: [{ required: true, message: I18N.get('from.UserEditForm.lastName.required') }],
      initialValue: user.profile.lastName,
    })
    const lastName_el = (
      <Input />
    )

    const bio_fn = getFieldDecorator('bio', {
      rules: [],
      initialValue: user.profile.bio,
    })

    const bio_el = (
      <Input.TextArea rows={4} placeholder={I18N.get('profile.skillsDetails.placeholder')} />
    )

    return {
      // General
      role: role_fn(role_el),
      firstName: firstName_fn(firstName_el),
      lastName: lastName_fn(lastName_el),
      bio: bio_fn(bio_el),
    }
  }

  renderHeader() {
    return (
      <h3 className="uef-header cr-title-with-bg">
        {I18N.get('profile.editBasicProfile')}
      </h3>
    )
  }

  renderBodySection() {
    const p = this.getInputProps()
    const formItemLayout = this.getFormItemLayout()

    return (
      <div className="uef-section-content">
        <FormItem label={I18N.get('from.UserEditForm.label.firstName')} {...formItemLayout}>
          {p.firstName}
        </FormItem>
        <FormItem label={I18N.get('from.UserEditForm.label.lastName')} {...formItemLayout}>
          {p.lastName}
        </FormItem>
        <FormItem label={I18N.get('user.edit.form.role')} {...formItemLayout}>
          {p.role}
        </FormItem>
        <FormItem label={I18N.get('from.UserEditForm.label.bio')} {...formItemLayout}>
          {p.bio}
        </FormItem>
      </div>
    )
  }

  renderAvatar(isMobile) {
    const p_avatar = {
      showUploadList: false,
      customRequest: (info) => {
        upload_file(info.file).then(async (d) => {
          await this.props.updateAvatar({
            profile: {
              avatar: d.url,
              avatarFilename: d.filename,
              avatarFileType: d.type,
            },
          })

          this.setState({ avatar: d.url })
          this.props.refetch()
        })
      },
    }

    return (
      <div className={`profile-avatar-container ${isMobile ? 'profile-avatar-container-mobile' : ''}`}>
        <div className="profile-avatar">
          <Upload
            name="avatar"
            listType="picture-card"
            className="avatar-uploader"
            showUploadList={false}
            {...p_avatar}
          >
            {this.props.avatar_loading
              ? (
                <div>
                  <Icon type="loading" />
                </div>
              )
              : (
                <img
                  src={this.getAvatarWithFallback(this.state.avatar)}
                  alt="avatar"
                  style={{ width: '100%' }}
                />
              )
            }
          </Upload>
        </div>
      </div>
    )
  }

  getAvatarWithFallback(avatar) {
    return _.isEmpty(avatar)
      ? USER_AVATAR_DEFAULT
      : avatar
  }

  renderSave() {
    return (
      <Button type="primary" htmlType="submit" loading={this.props.loading}>
        {I18N.get('profile.save')}
      </Button>
    )
  }

  ord_render() {
    return (
      <div className="c_userEditFormContainer">
        {this.renderHeader()}
        <Row gutter={8}>
          <Col span={18}>
            <Form onSubmit={this.handleSubmit.bind(this)}>
              {this.renderBodySection()}
              <FormItem className="uef-button-row">
                { this.renderSave() }
              </FormItem>
            </Form>
          </Col>
          <Col span={6}>
            {this.renderAvatar()}
          </Col>
        </Row>
      </div>
    )
  }
}
export default Form.create()(C)
