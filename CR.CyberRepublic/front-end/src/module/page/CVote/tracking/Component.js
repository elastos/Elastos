import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import _ from 'lodash'
import styled from 'styled-components'
import { CVOTE_TRACKING_STATUS, CVOTE_STATUS } from '@/constant'
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
    const { proposal, currentUserId, latestStatus } = this.props
    const isOwner = _.get(proposal, 'proposer._id') === currentUserId
    const notInReviewing = latestStatus !== CVOTE_TRACKING_STATUS.REVIEWING
    const notFinal = proposal.status !== CVOTE_STATUS.FINAL

    return isOwner && notInReviewing && notFinal && <CreateForm proposal={proposal} />
  }
}

export const Container = styled.div`
`
