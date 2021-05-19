import React from 'react'
import _ from 'lodash'
import I18N from '@/I18N'
import BaseComponent from '@/model/BaseComponent'
import ListItem from '../listItem/Container'

import { Container, List } from './style'

export default class extends BaseComponent {
  ord_render() {
    const listNode = this.renderList()
    const headerNode = this.renderHeader()
    return (
      <Container>
        {headerNode}
        {listNode}
      </Container>
    )
  }

  renderHeader() {
    return (
      <h2 className="title komu-a">{this.props.header || I18N.get('permission.title').toUpperCase()}</h2>
    )
  }

  renderList() {
    const { dataList } = this.props
    const result = _.map(dataList, data => this.renderItem(data))
    return <List>{result}</List>
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
