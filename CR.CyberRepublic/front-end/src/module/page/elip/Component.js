import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import StandardPage from '@/module/page/StandardPage'
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
      <div>
        <ElipList />
        <Footer />
      </div>
    )
  }
}
