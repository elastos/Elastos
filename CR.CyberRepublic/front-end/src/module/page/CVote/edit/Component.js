import React from 'react'
import StandardPage from '../../StandardPage'
import CVoteForm from '@/module/form/CVoteForm/Container'
import I18N from '@/I18N'
import BackLink from '@/module/shared/BackLink/Component'
import Footer from '@/module/layout/Footer/Container'

import { Container } from './style'

export default class extends StandardPage {
  constructor(p) {
    super(p)
    this.state.data = null
  }

  async componentDidMount() {
    const data = await this.props.getData(this.props.match.params.id)
    this.setState({ data })
  }

  ord_renderContent() {
    const { data } = this.state
    if (!data) return null
    const form = this.renderForm()
    return (
      <div>
        <Container>
          <BackLink link={`/proposals/${data._id}`} />
          {form}
        </Container>
        <Footer />
      </div>
    )
  }

  renderForm() {
    const props = {
      ...this.props,
      edit: this.props.match.params.id,
      data: this.state.data,
      onEdit: this.onEdit,
      onCancel: this.onCancel,
      header: I18N.get('council.voting.btnText.editProposal'),
    }
    return <CVoteForm {...props} />
  }

  onEdit = () => {
    const { data } = this.state
    this.props.history.push(`/proposals/${data._id}`)
  }

  onCancel = () => {
    this.onEdit()
  }
}
