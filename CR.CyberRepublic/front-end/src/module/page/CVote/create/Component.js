import React from 'react'
import StandardPage from '../../StandardPage'
import CVoteForm from '@/module/form/CVoteForm/Container'
import I18N from '@/I18N'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'

import { Container } from './style'

export default class extends StandardPage {
  constructor(props) {
    super(props)
    this.state = {
      creating: false,
    }
  }

  ord_renderContent() {
    const form = this.renderForm()
    return (
      <div>
        <Container>
          <BackLink link="/proposals" />
          {form}
        </Container>
        <Footer />
      </div>
    )
  }

  renderForm() {
    const props = {
      ...this.props,
      onCreated: this.onCreated,
      onCancel: this.onCancel,
      header: I18N.get('from.CVoteForm.button.add'),
    }
    return <CVoteForm {...props} />
  }

  onCreated = () => {
    this.props.history.push('/proposals')
  }

  onCancel = () => {
    this.onCreated()
  }
}
