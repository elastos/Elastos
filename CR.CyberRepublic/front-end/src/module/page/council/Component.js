import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import './style.scss'
import StandardPage from '../StandardPage'
import CVoteList from '../CVote/list/Container'

/**
 * TODO: all the positions should load from the DB, copy pasting for now
 * until applications are being processed
 */
export default class extends StandardPage {
  constructor(props) {
    super(props)

    this.state = {

      loading: false,
    }
  }

  ord_renderContent() {
    return (
      <div className="p_council">
        <CVoteList />
        <Footer />
      </div>
    )
  }
}
