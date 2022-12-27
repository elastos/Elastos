import React from 'react'
import I18N from '@/I18N'
import StandardPage from '../../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import OrganizerAppForm from '@/module/form/OrganizerAppForm/Container'

export default class extends StandardPage {

  ord_renderContent () {

    return (
      <div className="p_Social">
        <div className="ebp-header-divider" />
        <div className="ebp-page-title">
          <h3>
            {I18N.get('formext.anni2018.organizer.title')}
          </h3>
        </div>
        <div className="ebp-page">
          <OrganizerAppForm campaign="organizerApp"/>
        </div>
        <Footer />
      </div>
    )
  }
}
