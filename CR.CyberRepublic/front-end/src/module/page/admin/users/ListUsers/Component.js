import React from 'react'
import _ from 'lodash'
import BaseComponent from '@/model/BaseComponent'
import { Table } from 'antd'
import I18N from '@/I18N'
import config from '@/config'

import './style.scss'

export default class extends BaseComponent {
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
      title: I18N.get('1208'),
      dataIndex: '_id',
      render: (id, item) => `${item.profile.firstName} ${item.profile.lastName}`,
    }, {
      title: I18N.get('1202'),
      dataIndex: 'email',
    }, {
      title: I18N.get('1209'),
      dataIndex: 'profile.country',
      render: geolocation => config.data.mappingCountryCodeToName[geolocation],
      sorter: (a, b) => config.data.mappingCountryCodeToName[a.profile.country].localeCompare(config.data.mappingCountryCodeToName[b.profile.country]),
    }, {
      title: I18N.get('1204'),
      dataIndex: 'role',
      render: (role) => {
        if (role === 'LEADER') {
          return _.capitalize('ORGANIZER')
        }

        return _.capitalize(role)
      },
    }]

    const data = this.props.users

    return (
      <Table
        columns={columns}
        dataSource={data}
        rowKey={record => record.username}
        loading={this.props.loading}
      />
    )
  }

  linkProfileInfo(userId) {
    this.props.history.push(`/admin/profile/${userId}`)
  }
}
