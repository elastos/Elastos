import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import CVoteForm from '@/module/form/CVoteForm/Container'
import I18N from '@/I18N'

import { Container } from './style'

export default class extends BaseComponent {
  constructor(p) {
    super(p)
    this.state.data = null
  }

  ord_render() {
    return (
      <Container>
        <CVoteForm
          header={I18N.get('council.voting.btnText.editProposal')}
          edit={this.props.match.params.id}
          data={this.state.data}
          onEdit={this.props.onEdit}
          onCancel={this.props.onCancel}
        />
      </Container>
    )
  }

  async componentDidMount() {
    const data = await this.props.getData(this.props.match.params.id)
    this.setState({ data })
  }
}
