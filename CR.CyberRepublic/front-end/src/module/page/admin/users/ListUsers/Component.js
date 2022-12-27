import React from 'react'
import _ from 'lodash'
import UserEditForm from '@/module/form/UserEditBasicForm/Container'
import BaseComponent from '@/model/BaseComponent'
import { Table, Modal, Icon } from 'antd'
import I18N from '@/I18N'
import config from '@/config'
import { USER_ROLE } from '@/constant'

import './style.scss'

export default class extends BaseComponent {
  constructor(props) {
    super(props)
    this.state = {
      editing: false,
    }
  }

  ord_render() {
    const columns = [{
      title: I18N.get('profile.detail.avatar'),
      dataIndex: 'profile.avatar',
      render: avatar => (avatar ? (
        <img src={avatar} style={{ height: 30, width: 30 }} alt="avatar" />
      ) : 'None'),
    }, {
      title: I18N.get('1201'),
      dataIndex: 'username',
      render: (username, record) => <a onClick={this.linkProfileInfo.bind(this, record._id)} className="tableLink">{username}</a>,
      sorter: (a, b) => a.username.localeCompare(b.username),
      defaultSortOrder: 'ascend',
    }, {
      title: <div style={{ whiteSpace: 'nowrap' }}>{I18N.get('1208')}</div>,
      dataIndex: '_id',
      render: (id, item) => `${item.profile.firstName} ${item.profile.lastName}`,
    }, {
      title: I18N.get('1202'),
      dataIndex: 'email',
    }, {
      title: I18N.get('1209'),
      dataIndex: 'profile.country',
      render: geolocation => config.data.mappingCountryCodeToName[geolocation],
      sorter: (a, b) => _.get(config.data.mappingCountryCodeToName, a.profile.country, '').localeCompare(_.get(config.data.mappingCountryCodeToName, b.profile.country, '')),
    }, {
      title: I18N.get('1204'),
      dataIndex: 'role',
      filters: _.map(USER_ROLE, value => ({ text: _.capitalize(value), value })),
      onFilter: (value, item) => item.role === value,
      render: (role) => {
        if (role === 'LEADER') {
          return _.capitalize('ORGANIZER')
        }

        return _.capitalize(role)
      },
    }, {
      title: I18N.get('project.detail.columns.action'),
      render: (item) => <Icon type="edit" onClick={() => this.switchEditMode(item)} />,
    }]

    const data = this.props.users
    const { user, editing } = this.state
    return (
      <div>
        <Table
          columns={columns}
          dataSource={data}
          rowKey={record => record.username}
          loading={this.props.loading}
        />
        {editing && this.renderEditForm(user)}
      </div>
    )
  }

  renderEditForm(user) {
    if (!user) return null
    return (
      <Modal
        className="project-detail-nobar"
        visible={this.state.editing}
        onOk={this.switchEditMode}
        onCancel={this.switchEditMode}
        footer={null}
        width="70%"
      >
        <UserEditForm
          user={user}
          refetch={this.props.refetch}
          switchEditMode={this.switchEditMode}
          completing={false}
        />
        <div style={{textAlign: 'center'}}>
          <a onClick={() => this.linkProfileInfo(user._id)} className="tableLink">{I18N.get('profile.editFullProfile')}</a>
        </div>
      </Modal>
    )
  }

  switchEditMode = (user) => {
    this.setState({
      editing: !this.state.editing,
      user,
    })
  }

  linkProfileInfo(userId) {
    this.props.history.push(`/admin/profile/${userId}`)
  }
}
