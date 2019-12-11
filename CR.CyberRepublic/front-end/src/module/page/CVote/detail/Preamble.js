import React from 'react'
import PropTypes from 'prop-types'
import moment from 'moment/moment'
import { Row, Col } from 'antd'
import I18N from '@/I18N'
import PopoverProfile from '@/module/common/PopoverProfile'
import styled from 'styled-components'

const Component = ({
  vid,
  title,
  proposer,
  type,
  status,
  createdAt,
  createdBy,
  reference,
  user
}) => {
  // header
  const headerNode = (
    <Header id="preamble">{I18N.get('proposal.fields.preamble')}</Header>
  )

  const typeMap = {
    1: I18N.get('council.voting.type.newMotion'),
    2: I18N.get('council.voting.type.motionAgainst'),
    3: I18N.get('council.voting.type.anythingElse')
  }
  // type
  const typeNode = (
    <Item>
      <Col span={6}>
        <ItemTitle>{I18N.get('proposal.fields.type')}</ItemTitle>
      </Col>
      <Col span={18}>
        <ItemText>{typeMap[type]}</ItemText>
      </Col>
    </Item>
  )
  // id
  const idNode = (
    <Item>
      <Col span={6}>
        <ItemTitle>
          {I18N.get('proposal.fields.preambleSub.proposal')}
        </ItemTitle>
      </Col>
      <Col span={18}>
        <ItemText>{`#${vid}`}</ItemText>
      </Col>
    </Item>
  )
  // title
  const titleNode = (
    <Item>
      <Col span={6}>
        <ItemTitle>{I18N.get('proposal.fields.preambleSub.title')}</ItemTitle>
      </Col>
      <Col span={18}>
        <ItemText>{title}</ItemText>
      </Col>
    </Item>
  )
  // proposer
  const proposerNode = (
    <Item>
      <Col span={6}>
        <ItemTitle>
          {I18N.get('proposal.fields.preambleSub.proposer')}
        </ItemTitle>
      </Col>
      <Col span={18}>
        <PopoverProfile owner={proposer} curUser={user} />
      </Col>
    </Item>
  )
  // referee
  const refereeNode = (
    <Item>
      <Col span={6}>
        <ItemTitle>
          {I18N.get('proposal.fields.preambleSub.referee')}
        </ItemTitle>
      </Col>
      <Col span={18}>
        <PopoverProfile owner={createdBy} curUser={user} />
      </Col>
    </Item>
  )
  // status
  const statusNode = (
    <Item>
      <Col span={6}>
        <ItemTitle>{I18N.get('proposal.fields.preambleSub.status')}</ItemTitle>
      </Col>
      <Col span={18}>
        <ItemText>{I18N.get(`cvoteStatus.${status}`)}</ItemText>
      </Col>
    </Item>
  )
  // created
  const createdNode = (
    <Item>
      <Col span={6}>
        <ItemTitle>{I18N.get('proposal.fields.preambleSub.created')}</ItemTitle>
      </Col>
      <Col span={18}>
        <ItemText>{moment(createdAt).format('MMM D, YYYY')}</ItemText>
      </Col>
    </Item>
  )
  return (
    <div>
      {headerNode}
      {idNode}
      {titleNode}
      {proposerNode}
      {reference && reference.displayId ? refereeNode : null}
      {typeNode}
      {statusNode}
      {createdNode}
    </div>
  )
}

const propTypes = {
  vid: PropTypes.number.isRequired,
  title: PropTypes.string.isRequired,
  proposer: PropTypes.object.isRequired,
  status: PropTypes.string.isRequired,
  createdAt: PropTypes.string.isRequired
}

Component.propTypes = propTypes

export default Component

const Header = styled.h4`
  font-size: 20px;
`
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
