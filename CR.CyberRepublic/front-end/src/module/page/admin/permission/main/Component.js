import React from 'react'
import _ from 'lodash'
import {
  Col, Row, Spin, Tabs, Modal, Button,
} from 'antd'
import I18N from '@/I18N'
import AdminPage from '../../BaseAdmin'
import Footer from '@/module/layout/Footer/Container'
import Navigator from '@/module/page/shared/HomeNavigator/Container'
import PermissionForm from '@/module/form/PermissionForm/Container'
import List from '../list/Container'
import {
  USER_ROLE_TO_TEXT,
  RESOURCE_TYPE_TO_TEXT,
} from '@/constant'

import '@/module/page/admin/admin.scss'

const TabPane = Tabs.TabPane

export default class extends AdminPage {
  state = {
    showForm: false,
  }

  componentDidMount() {
    super.componentDidMount()

    this.refetch()
  }

  componentWillUnmount() {
    this.props.resetAll()
  }

  ord_renderContent() {
    const { dataList, loading, loadingForRole, currentUserId } = this.props
    const loadingNode = <div className="center"><Spin size="large" /></div>
    let listNode = loading || loadingForRole ? loadingNode : this.renderTabs()

    if (_.isEmpty(dataList) && !loading || !currentUserId) {
      listNode = <div className="center">{I18N.get('suggestion.nodata')}</div>
    }

    const headerNode = this.renderHeader()
    const createBtn = this.renderCreateButton()
    const createForm = this.renderCreateForm()
    return (
      <div>
        <div className="p_AdminPermissionList ebp-wrap">
          <Row>
            <Col sm={24} md={4} className="wrap-box-navigator">
              <Navigator selectedItem="profileAdminPermission" />
            </Col>
            <Col sm={24} md={20} className="c_ProfileContainer admin-right-column wrap-box-user">
              {headerNode}
              {createBtn}
              {listNode}
              {createForm}
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

  renderCreateButton() {
    return (
      <div>
        <Button onClick={this.showCreateForm}>
          Create Permission
        </Button>
        <span style={{ marginLeft: '10px' }}>(Require server to restart to take effect)</span>
      </div>
    )
  }

  onFormSubmit = async (param) => {
    try {
      await this.props.create(param)
      this.showCreateForm()
      this.refetch()
    } catch (error) {
      // console.log(error)
    }
  }

  renderCreateForm = () => {
    const props = {
      onFormCancel: this.showCreateForm,
      onFormSubmit: this.onFormSubmit,
    }

    return (
      <Modal
        className="project-detail-nobar"
        maskClosable={false}
        visible={this.state.showForm}
        onOk={this.showCreateForm}
        onCancel={this.showCreateForm}
        footer={null}
        width="70%"
      >
        <PermissionForm {...props} />
      </Modal>
    )
  }

  showCreateForm = () => {
    const { showForm } = this.state
    this.setState({
      showForm: !showForm,
    })
  }

  /**
   * Refetch the data based on the current state retrieved from getQuery
   */
  refetch = () => {
    this.props.getList()
    this.props.getListForRole()
  }
}
