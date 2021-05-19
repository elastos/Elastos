import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import { Spin } from 'antd'
import URI from 'urijs'
import StandardPage from '../StandardPage'

import './style.scss'

export default class extends StandardPage {
  ord_states() {
    return {
      loading: false,
      loadingError: false
    }
  }

  ord_renderContent () {
    return (
      <div className="p_sso">
        <div className="ebp-header-divider" />
        <div className="ebp-page">
          {
            !this.state.loadingError ? this.renderLoading() : this.renderError()
          }
        </div>
        <Footer />
      </div>
    )
  }

  renderLoading() {
    return (
      <div className="loader flex-center">
        <div className="msg">
                    We are logging you in to the forum
        </div>
        <Spin size="large" />
      </div>
    )
  }

  renderError() {
    return (
      <div className="loader flex-center">
        <div className="msg">
                There was an error with your login token - please contact us at
          {' '}
          <a href="mailto:support@cyberrepublic.org">support@cyberrepublic.org</a>
        </div>
      </div>
    )
  }

  /**
     * This page is only loaded from an SSO request, if the sso token is invalid
     * we should explain SSO and prompt the user to create an account or login
     *
     * @returns {Promise<void>}
     */
  async componentDidMount() {
    const params = new URI(this.props.location.search || '').search(true)
    const { SSO_URL, FORUM_URL } = process.env
    let loginStr

    this.setState({ loading: true })

    // called sso/login with improper params, should never happen
    if (!params.sso || !params.sig) {
      this.setState({
        loadingError: true
      })
      return
    }

    try {
      const result = await this.props.getLoginStr(params)
      loginStr = `${SSO_URL}?${result.url}`
      // console.log('loginstr: ', loginStr, SSO_URL, FORUM_URL);

      window.location.href = loginStr

    } catch (err) {

      console.log(err)

      // no login found, there are two possibilities here
      // 1. if the user is logged in, there is an error on our end since an SSO token was passed in and we didn't match a user
      // 2. the user went to the forum first but wasn't logged in on CR, then we just prompt to login
      if (this.props.isLogin) {
        this.setState({
          loadingError: true
        })
        return
      }

      // 2. redirect to login and notify user they need to be logged in
      sessionStorage.setItem('loginRedirect', window.location.pathname + window.location.search)
      window.location.href = '/login?MSG_CODE=1'
    }

  }
}
