import React from 'react'
import StandardPage from '../StandardPage'
import ForgotPasswordForm from '@/module/form/ForgotPasswordForm/Container'
import I18N from '@/I18N'

import './style.scss'

export default class extends StandardPage {

  ord_renderContent() {
    return (
      <div className="p_forgotPassword ebp-wrap">
        <div className="d_box">
          <p>
            {I18N.get('forgot.title')}
          </p>
          <ForgotPasswordForm />
        </div>
      </div>
    )
  }
}
