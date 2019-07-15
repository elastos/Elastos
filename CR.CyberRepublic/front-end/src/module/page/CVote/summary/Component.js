import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import _ from 'lodash'
import styled from 'styled-components'
import Summary from './list/Container'
import CreateForm from './create/Container'

export default class extends BaseComponent {
  ord_render() {
    return (
      <Container>
        {this.renderSummary()}
        {this.renderForm()}
      </Container>
    )
  }

  renderSummary() {
    const { proposal } = this.props
    return <Summary proposal={proposal} />
  }

  renderForm() {
    const { proposal, currentUserId } = this.props
    const isOwner = _.get(proposal, 'proposer._id') === currentUserId
    return isOwner && <CreateForm proposal={proposal} />
  }
}

export const Container = styled.div`
`
