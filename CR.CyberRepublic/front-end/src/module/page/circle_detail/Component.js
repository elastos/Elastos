import React from 'react'
import StandardPage from '../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import CircleDetail from './detail/Container'
import './style.scss'

export default class extends StandardPage {
  ord_renderContent () {
    return (
      <div className="p_CircleDetail">
        <div className="ebp-header-divider" />
        <div className="p_admin_index">
          <div className="d_box">
            <div className="p_admin_content">
              <CircleDetail circleId={this.props.match.params.circleId}/>
            </div>
          </div>
        </div>
        <Footer/>
      </div>
    )
  }
}
