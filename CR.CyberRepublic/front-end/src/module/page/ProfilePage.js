import React from 'react'
import StandardPage from './StandardPage'

export default class extends StandardPage {

  ord_checkLogin(isLogin) {
    if (!isLogin) {
      this.props.history.replace('/login')
    }
  }
}
