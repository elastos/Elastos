import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import './style.scss'

import StandardPage from '../StandardPage'
import ProjectDetail from './detail/Container'

export default class extends StandardPage {
  ord_renderContent() {
    return (
      <div className="p_ProjectDetail">
        <div className="ebp-header-divider" />
        <div>
          <div className="d_box">
            <div className="p_admin_content">
              <ProjectDetail taskId={this.props.match.params.taskId} />
            </div>
          </div>
        </div>
        <Footer />
      </div>
    )
  }
}
