import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import './style.scss'
import StandardPage from '../StandardPage'

export default class extends StandardPage {

  ord_renderContent () {

    return (
      <div className="p_Leader">
        <div className="ebp-header-divider" />
        <div className="ebp-page">
          <div className="ebp-page-title">
            <h1>
                            Elastos Leader Program
            </h1>
          </div>

                    TODO: description of leader program, voting process, EVP and signup
        </div>
        <Footer />
      </div>
    )
  }
}
