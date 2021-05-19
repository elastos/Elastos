import React from 'react'
import I18N from '@/I18N'
import StandardPage from '../../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import Training1Form from '@/module/form/Training1Form/Container'

export default class extends StandardPage {

  ord_renderContent () {

    return (
      <div className="p_Social">
        <div className="ebp-header-divider" />
        <div className="ebp-page-title">
          <h2>
            {I18N.get('formext.anni2018.training.title')}
          </h2>
        </div>
        <div className="ebp-page">
          <Training1Form campaign="training1"/>
        </div>
        <Footer />
      </div>
    )
  }
}
