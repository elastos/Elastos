import React from 'react'
import BasePage from '@/model/BasePage'
import Meta from '@/module/common/Meta'

export default class extends BasePage {
  ord_renderPage() {
    return (
      <div className="p_emptyPage">
        <Meta />
        {this.ord_renderContent()}
      </div>
    )
  }

  ord_renderContent() {
    return null
  }
}
