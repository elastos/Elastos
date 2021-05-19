import React from 'react'
import StandardPage from '../StandardPage'
import ResetPasswordForm from '@/module/form/ResetPasswordForm/Container'
import I18N from '@/I18N'

import './style.scss'

export default class extends StandardPage {

  ord_renderContent() {
    return (
      <div className="p_resetPassword ebp-wrap">
        <div className="d_box">
          <p>
            {I18N.get('forgot.new_password')}
          </p>
          <ResetPasswordForm />
        </div>
      </div>
    )
  }
}
