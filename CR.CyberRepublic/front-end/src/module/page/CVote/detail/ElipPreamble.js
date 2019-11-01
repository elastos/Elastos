import React from 'react'
import PropTypes from 'prop-types'
import I18N from '@/I18N'
import { Row, Col } from 'antd'
import moment from 'moment/moment'
import _ from 'lodash'
import userUtil from '@/util/user'

import styled from 'styled-components'
import {
  Part,
  PartTitle,
  PartContent
} from './style'

const Component = ({
  vid: proposal,
  title,
  proposedBy,
  proposer,
  type,
  status,
  referee,
  createdAt: created,
  createdBy,
}) => {
  const result = {proposal, title, proposer, referee, type, status, created}
  const typeMap = {
    4: I18N.get('council.voting.type.standardTrack'),
    5: I18N.get('council.voting.type.information'),
    6: I18N.get('council.voting.type.process'),
  }
  const proposalValue = proposal && `#${proposal}`
  const proposerValue = `${proposedBy} <${_.get(proposer, 'email')}>`
  const refereeValue = referee && `${userUtil.formatUsername(createdBy)} <${_.get(createdBy, 'email')}>`
  const preambles = {...result, proposal: proposalValue, proposer: proposerValue, referee: refereeValue, type: typeMap[result.type], created: moment(created).format('MMM D, YYYY')}
  const itemFunction = (key, value) => (
    <Item key={key}>
      <Col span={6}>
        <ItemTitle>{key}</ItemTitle>
      </Col>
      <Col span={18}>
        <ItemText className="preamble preamble-value">{value}</ItemText>
      </Col>
    </Item>
  )
  return (
    <Part id="preamble">
      <PartTitle>{I18N.get('elip.fields.preamble')}</PartTitle>
      <PartContent className="preamble">
        {_.map(
          preambles,
          (v, k) =>
            !_.isEmpty(v) &&
            itemFunction(
              k === 'type'
                ? I18N.get('proposal.fields.type')
                : I18N.get(`proposal.fields.preambleSub.${k}`),
              v
            )
        )}
      </PartContent>
    </Part>
  )
}

const propTypes = {
  vid: PropTypes.number.isRequired,
  title: PropTypes.string.isRequired,
  proposedBy: PropTypes.string.isRequired,
  proposer: PropTypes.object.isRequired,
  referee: PropTypes.object,
  status: PropTypes.string.isRequired,
  type: PropTypes.string.isRequired,
  createdAt: PropTypes.string.isRequired,
  createdBy: PropTypes.object.isRequired,
}

Component.propTypes = propTypes

export default Component

const Item = styled(Row)`
  margin-top: 10px;
  font-size: 13px;
  font-style: italic;
`
const ItemTitle = styled.div`
  font-weight: 400;
  :after {
    content: ':';
  }
`
const ItemText = styled.div`
  font-weight: 200;
`
