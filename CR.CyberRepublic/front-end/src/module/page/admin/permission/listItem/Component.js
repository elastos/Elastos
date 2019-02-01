import React from 'react';
import _ from 'lodash'
import { Spin, Switch } from 'antd'
import I18N from '@/I18N'
import BaseComponent from '@/model/BaseComponent'

import './style.scss'

export default class extends BaseComponent {
  constructor(props) {
    super(props)

    // we use the props from the redux store if its retained
    this.state = {
      showMobile: false,
      role: 'ADMIN',
    }
  }

  // componentDidMount() {
  //   this.refetch()
  // }

  // componentWillUnmount() {
  //   this.props.resetAll()
  // }

  ord_render() {
    const { permission } = this.props;
    const desc = <div className="desc">{permission.desc}</div>
    const switchNode = this.renderSwitch(permission)
    return (
      <div key={permission._id} className="c_PermissionListItem">
        {desc}
        {switchNode}
      </div>
    )
  }

  renderSwitch = () => {
    const { permissionRole } = this.props
    const defaultChecked = _.get(permissionRole, 'isAllowed', false)

    return <Switch defaultChecked={defaultChecked} onChange={checked => this.onChange(checked, _.get(permissionRole, '_id'))} />
  }

  onChange = (checked, id) => {
    console.log(`switch to ${checked}`);
    const { updateForRole, permission, role } = this.props
    const { _id: permissionId, resourceType, url, httpMethod } = permission
    const updateObject = {
      id,
      isAllowed: checked,
      role,
      permissionId,
      resourceType,
      url,
      httpMethod,
    }
    updateForRole(updateObject)
  }

  /**
   * Builds the query from the current state
   */
  getQuery = () => {
    const { permission, role, permissionRole } = this.props
    const { _id: permissionId, resourceType, url, httpMethod } = permission
    const query = {
      // id: _.get(permissionRole, '_id'),
      role,
      permissionId,
      resourceType,
      url,
      httpMethod,
    }

    return query
  }

  /**
   * Refetch the permission based on the current state retrieved from getQuery
   */
  refetch = () => {
    const query = this.getQuery()
    this.props.getDetail(query)
  }
}
