import React from 'react'
import PropTypes from 'prop-types'
import I18N from '@/I18N'
import { Row, Col } from 'antd'
import moment from 'moment/moment'
import _ from 'lodash'
import PopoverProfile from '@/module/common/PopoverProfile'

import styled from 'styled-components'
import {
  Part,
  PartTitle,
  PartContent
} from './style'

const Component = ({
  vid: proposal,
  title,
  proposer,
  type,
  status,
  createdAt: created,
  user
}) => {
  const result = {proposal, title, proposer, type, status, created}
  const typeMap = {
    4: I18N.get('council.voting.type.standardTrack'),
    5: I18N.get('council.voting.type.process'),
    6: I18N.get('council.voting.type.information'),
  }
  const proposalValue = proposal && `#${proposal}`
  const proposerValue = <PopoverProfile owner={proposer} curUser={user} />
  const statusValue = I18N.get(`cvoteStatus.${status}`)
  const preambles = {...result, proposal: proposalValue, proposer: proposerValue, type: typeMap[result.type], status: statusValue, created: moment(created).format('MMM D, YYYY')}
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
  proposer: PropTypes.object.isRequired,
  status: PropTypes.string.isRequired,
  type: PropTypes.string.isRequired,
  createdAt: PropTypes.string.isRequired,
  user: PropTypes.object.isRequired,
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
