import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import _ from 'lodash'
import styled from 'styled-components'
import Tracking from './list/Container'
import CreateForm from './create/Container'

export default class extends BaseComponent {
  ord_render() {
    return (
      <Container>
        {this.renderTracking()}
        {this.renderForm()}
      </Container>
    )
  }

  renderTracking() {
    const { proposal } = this.props
    return <Tracking proposal={proposal} />
  }

  renderForm() {
    const { proposal, currentUserId } = this.props
    const isOwner = _.get(proposal, 'proposer._id') === currentUserId
    return isOwner && <CreateForm proposal={proposal} />
  }
}

export const Container = styled.div`
`
