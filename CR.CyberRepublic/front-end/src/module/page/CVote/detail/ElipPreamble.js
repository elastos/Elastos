import React from 'react'
import PropTypes from 'prop-types'
import I18N from '@/I18N'
import { Row, Col } from 'antd'
import moment from 'moment/moment'
import _ from 'lodash'

import styled from 'styled-components'
import {
  Part,
  PartTitle,
  PartContent
} from './style'

const Component = ({
  vid: elip,
  title,
  proposedBy: author,
  discussions,
  status,
  type,
  createdAt: created,
  requires,
  replaces,
  superseded
}) => {
  const result = {elip, title, author, discussions, status, type, created, requires, replaces, superseded}
  const typeMap = {
    4: I18N.get('council.voting.type.standardTrack'),
    5: I18N.get('council.voting.type.information'),
    6: I18N.get('council.voting.type.process'),
  }
  const preambles = {...result, type: typeMap[result.type], created: moment(created).format('MMM D, YYYY')}
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
        {_.map(preambles, (v, k) => !_.isEmpty(v) && itemFunction(I18N.get(`elip.fields.preambleItems.${k}`), v))}
      </PartContent>
    </Part>
  )
}

const propTypes = {
  vid: PropTypes.number.isRequired,
  title: PropTypes.string.isRequired,
  proposedBy: PropTypes.string.isRequired,
  discussions: PropTypes.string,
  status: PropTypes.string.isRequired,
  type: PropTypes.string.isRequired,
  createdAt: PropTypes.string.isRequired,
  requires: PropTypes.string,
  replaces: PropTypes.string,
  superseded: PropTypes.string
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
