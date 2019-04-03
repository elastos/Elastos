import React from 'react'
import Navigator from '@/module/page/shared/HomeNavigator/Container'
import I18N from '@/I18N'
import SubmissionCreateForm from '@/module/form/SubmissionCreateForm/Container'
import { Col, Row } from 'antd'

import ProfilePage from '../../ProfilePage'
import '../../admin/admin.scss'

export default class extends ProfilePage {
  ord_renderContent () {
    return (
      <div className="c_ProfileContainer">
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_admin_breadcrumb">
              <br/>
            </div>
            <div className="p_ProfileTeams p_admin_content">
              <Row>
                <Col sm={24} md={4} className="admin-left-column wrap-box-navigator">
                  <Navigator selectedItem="profileSubmissions"/>
                </Col>
                <Col sm={24} md={20} className="admin-right-column wrap-box-user">
                  <h4 className="p_profile_action_title">{I18N.get('profile.submission.create')}</h4>
                  <SubmissionCreateForm />
                </Col>
              </Row>
            </div>
          </div>
        </div>
      </div>
    )
  }
}
