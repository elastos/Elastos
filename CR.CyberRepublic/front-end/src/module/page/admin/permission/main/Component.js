import React from 'react'
import _ from 'lodash'
import {
  Col, Row, Spin, Tabs,
} from 'antd'
import I18N from '@/I18N'
import AdminPage from '../../BaseAdmin'
import Footer from '@/module/layout/Footer/Container'
import Navigator from '@/module/page/shared/HomeNavigator/Container'
import List from '../list/Container'
import {
  USER_ROLE_TO_TEXT,
  RESOURCE_TYPE_TO_TEXT,
} from '@/constant'

import '@/module/page/admin/admin.scss'

const TabPane = Tabs.TabPane

export default class extends AdminPage {
  componentDidMount() {
    super.componentDidMount()

    this.refetch()
  }

  componentWillUnmount() {
    this.props.resetAll()
  }

  ord_renderContent() {
    const { dataList, loading, loadingForRole, currentUserId } = this.props;
    const loadingNode = <div className="center"><Spin size="large" /></div>
    let listNode = loading || loadingForRole ? loadingNode : this.renderTabs()

    if (_.isEmpty(dataList) && !loading || !currentUserId) {
      listNode = <div className="center">{I18N.get('suggestion.nodata')}</div>
    }

    const headerNode = this.renderHeader()
    return (
      <div>
        <div className="p_AdminPermissionList ebp-wrap">
          <Row>
            <Col sm={24} md={4} className="wrap-box-navigator">
              <Navigator selectedItem="profileAdminPermission" />
            </Col>
            <Col sm={24} md={20} className="c_ProfileContainer admin-right-column wrap-box-user">
              {headerNode}
              {listNode}
            </Col>
          </Row>
        </div>
        <Footer />
      </div>
    )
  }

  renderTabs() {
    const roles = _.keys(USER_ROLE_TO_TEXT)
    const { dataList } = this.props
    const accuData = _.reduce(dataList, (prev, curr) => {
      if (!_.isEmpty(prev[curr.resourceType])) {
        prev[curr.resourceType].push(curr)
        return prev
      }
      return _.extend(prev, { [curr.resourceType]: [curr] })
    }, {})
    const panes = _.map(roles, role => <TabPane tab={USER_ROLE_TO_TEXT[role]} key={role}>{this.renderList(role, accuData)}</TabPane>)
    return (
      <Tabs defaultActiveKey={roles[0]}>
        {panes}
      </Tabs>
    )
  }

  getDataList = (resourceType) => {
    const { dataList } = this.props
    return _.filter(dataList, data => data.resourceType === resourceType)
  }

  getDataListForRole = (resourceType, role) => {
    const { dataListForRole } = this.props
    return _.filter(dataListForRole, data => data.resourceType === resourceType && data.role === role)
  }

  renderList(role, accuData) {
    const nodes = _.map(accuData, (dataList, resourceType) => {
      const dataListForRole = this.getDataListForRole(resourceType, role)
      const header = RESOURCE_TYPE_TO_TEXT[resourceType] || resourceType
      const props = {
        dataList,
        dataListForRole,
        role,
        header,
      }
      return (
        <Col lg={8} md={12} sm={24} key={resourceType}>
          <List {...props} />
        </Col>
      )
    })

    return (
      <Row gutter={16}>
        {nodes}
      </Row>
    )
  }

  renderHeader() {
    return (
      <h2 className="title komu-a cr-title-with-icon">{this.props.header || I18N.get('permission.title').toUpperCase()}</h2>
    )
  }

  /**
   * Refetch the data based on the current state retrieved from getQuery
   */
  refetch = () => {
    this.props.getList()
    this.props.getListForRole()
  }
}
