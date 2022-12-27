import React from 'react'
import StandardPage from '../StandardPage'
import Footer from '@/module/layout/Footer/Container'

import PublicProfileDetail from '@/module/profile/detail/Container'

import './style.scss'

export default class extends StandardPage {
  ord_renderContent() {
    return (
      <div className="p_Member">
        <div className="ebp-header-divider" />
        <div className="ebp-page">
          <PublicProfileDetail userId={this.props.match.params.userId} />
        </div>
        <Footer />
      </div>
    )
  }
}
