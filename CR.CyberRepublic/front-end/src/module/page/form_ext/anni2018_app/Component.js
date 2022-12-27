import React from 'react'
import I18N from '@/I18N'
import {Row, Col} from 'antd'
import StandardPage from '../../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import AnniversaryAppForm from '@/module/form/AnniversaryEventForm/Container'

export default class extends StandardPage {

  ord_renderContent () {

    return (
      <div className="p_Social">
        <div className="ebp-header-divider" />
        <div className="ebp-page-title">
          <h3>
            {I18N.get('formext.anni2018.app.title')}
          </h3>
        </div>
        <div className="ebp-page">
          <AnniversaryAppForm/>
        </div>
        <Footer />
      </div>
    )
  }
}
