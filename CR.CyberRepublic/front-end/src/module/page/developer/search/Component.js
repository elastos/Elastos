import React from 'react'
import { Row } from 'antd'
import StandardPage from '../../StandardPage'
import Search from '@/module/search/Container'
import './style.scss'

export default class extends StandardPage {
  ord_renderContent () {
    return (
      <div className="p_DeveloperSearch">
        <div className="ebp-header-divider" />
        <div className="ebp-wrap">
          <Row className="d_row">
            <Search/>
          </Row>
        </div>
      </div>
    )
  }
}
