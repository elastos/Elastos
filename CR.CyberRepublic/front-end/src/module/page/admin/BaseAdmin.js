import React from 'react'
import StandardPage from '../StandardPage'

export default class extends StandardPage {

  ord_checkLogin(isLogin, isAdmin) {
    if (!isLogin || !isAdmin) {
      this.props.history.replace('/profile/info')
    }
  }
}
