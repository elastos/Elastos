import React from 'react'
import StandardPage from '../StandardPage'
import I18N from '@/I18N'

export default class extends StandardPage {
  ord_renderContent() {
    return (
      <div className="ebp-page">
        <h1>{I18N.get('error.notfound')}</h1>
      </div>

    )
  }
}
