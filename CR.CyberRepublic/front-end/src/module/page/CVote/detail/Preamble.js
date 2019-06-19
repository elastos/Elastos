import React from 'react'
import PropTypes from 'prop-types'
import moment from 'moment/moment'
import { Row, Col } from 'antd'
import I18N from '@/I18N'

import styled from 'styled-components'

const Component = ({ vid, title, proposedBy, proposedByEmail, status, createdAt }) => {
  // header
  const headerNode = <Header id="preamble">{I18N.get('proposal.fields.preamble')}</Header>
  // note
  // const noteNode = <Note>Note: For confidential purpose, no one elase including council members is not able to access these files untill the bidding ended.</Note>
  // id
  const idNode = (
    <Item>
      <Col span={6}><ItemTitle>{I18N.get('proposal.fields.preambleSub.proposal')}</ItemTitle></Col>
      <Col span={18}><ItemText>{`#${vid}`}</ItemText></Col>
    </Item>
  )
  // title
  const titleNode = (
    <Item>
      <Col span={6}><ItemTitle>{I18N.get('proposal.fields.preambleSub.title')}</ItemTitle></Col>
      <Col span={18}><ItemText>{title}</ItemText></Col>
    </Item>
  )
  // proposer
  const proposerNode = (
    <Item>
      <Col span={6}><ItemTitle>{I18N.get('proposal.fields.preambleSub.proposer')}</ItemTitle></Col>
      <Col span={18}><ItemText>{`${proposedBy} <${proposedByEmail}>`}</ItemText></Col>
    </Item>
  )
  // status
  const statusNode = (
    <Item>
      <Col span={6}><ItemTitle>{I18N.get('proposal.fields.preambleSub.status')}</ItemTitle></Col>
      <Col span={18}><ItemText>{I18N.get(`cvoteStatus.${status}`)}</ItemText></Col>
    </Item>
  )
  // created
  const createdNode = (
    <Item>
      <Col span={6}><ItemTitle>{I18N.get('proposal.fields.preambleSub.created')}</ItemTitle></Col>
      <Col span={18}><ItemText>{moment(createdAt).format('MMM D, YYYY')}</ItemText></Col>
    </Item>
  )
  return (
    <div>
      {headerNode}
      {/* {noteNode} */}
      {idNode}
      {titleNode}
      {proposerNode}
      {statusNode}
      {createdNode}
    </div>
  )
}

const propTypes = {
  vid: PropTypes.number.isRequired,
  title: PropTypes.string.isRequired,
  proposedBy: PropTypes.string.isRequired,
  proposedByEmail: PropTypes.string.isRequired,
  status: PropTypes.string.isRequired,
  createdAt: PropTypes.string.isRequired,
}

Component.propTypes = propTypes


export default Component

const Header = styled.h4`
  font-size: 20px;
`
const Note = styled.div`
  font-weight: 200;
`
const Item = styled(Row)`
  margin-top: 10px;
  font-size: 13px;
`
const ItemTitle = styled.div`
  font-weight: 400;
`
const ItemText = styled.div`
  font-weight: 200;
  font-style: italic;
`
