import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import StandardPage from '@/module/page/StandardPage'
import Meta from '@/module/common/Meta'
import ElipList from './list/Container'

export default class extends StandardPage {
  constructor(props) {
    super(props)
    this.state = {
      loading: false
    }
  }

  ord_renderContent() {
    return (
      <React.Fragment>
        <Meta
          title="Cyber Republic - Elastos"
          url="/elips"
        />
        <ElipList />
        <Footer />
      </React.Fragment>
    )
  }
}
