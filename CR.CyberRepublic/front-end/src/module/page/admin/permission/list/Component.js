import React from 'react';
import _ from 'lodash'
import { Spin } from 'antd'
import I18N from '@/I18N'
import BaseComponent from '@/model/BaseComponent'
import ListItem from '../listItem/Container'

import './style.scss'

export default class extends BaseComponent {
  ord_render() {
    const listNode = this.renderList()
    const headerNode = this.renderHeader()
    return (
      <div className="p_PermissionList">
        {headerNode}
        {listNode}
      </div>
    )
  }

  renderHeader() {
    return (
      <div className="cr-permission-header">
        <h2 className="title komu-a">{this.props.header || I18N.get('permission.title').toUpperCase()}</h2>
      </div>
    )
  }

  renderList() {
    const { dataList } = this.props
    const result = _.map(dataList, data => this.renderItem(data))
    return <div className="list-container">{result}</div>
  }

  renderItem = (data) => {
    const { dataListForRole, role } = this.props
    const { _id: id } = data
    const permissionRole = _.find(dataListForRole, perm => perm.permissionId === id)
    const props = {
      permissionRole,
      permission: data,
      role,
      key: id,
    }
    return <ListItem {...props} />
  }
}
