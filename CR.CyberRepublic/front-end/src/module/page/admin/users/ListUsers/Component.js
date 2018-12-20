import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Table, Icon } from 'antd'
import I18N from '@/I18N'
import config from '@/config'

import './style.scss'
import moment from 'moment/moment'

export default class extends BaseComponent {

    ord_render () {
        const columns = [{
            title: I18N.get('1201'),
            dataIndex: 'username',
            render: (username, record) => {
                return <a onClick={this.linkProfileInfo.bind(this, record._id)} className="tableLink">{username}</a>
            },
            sorter: (a, b) => {
                return a.username.localeCompare(b.username)
            },
            defaultSortOrder: 'ascend'
        }, {
            title: I18N.get('1208'),
            dataIndex: '_id',
            render: (id, item) => {
                return item.profile.firstName + ' ' + item.profile.lastName
            }

        }, {
            title: I18N.get('1202'),
            dataIndex: 'email',
        }, {
            title: I18N.get('1209'),
            dataIndex: 'profile.country',
            render: (geolocation, item) => {
                return config.data.mappingCountryCodeToName[geolocation]
            },
            sorter: (a, b) => {
                return config.data.mappingCountryCodeToName[a.profile.country].localeCompare(config.data.mappingCountryCodeToName[b.profile.country])
            }
        }, {
            title: I18N.get('1204'),
            dataIndex: 'role',
            render: (role) => {
                if (role === 'LEADER') {
                    role = 'ORGANIZER'
                }

                return _.capitalize(role)
            }
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
