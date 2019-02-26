import React from 'react'
import PropTypes from 'prop-types'
import { Row, Col } from 'antd'
import _ from 'lodash'
import { CVOTE_RESULT_TEXT } from '@/constant'

import { Container, RejectRow, Reason, Label, List, Item, Avatar } from './style'

const Component = ({ label, type, dataList }) => {
  const votesNode = _.map(dataList, (data, key) => {
    const isReject = type === CVOTE_RESULT_TEXT.reject.toLowerCase()
    const userNode = (
      <Item key={key}>
        <Avatar src={data.avatar} alt="voter avatar" />
        <div>{data.name}</div>
      </Item>
    )

    if (!isReject) return userNode

    const reasonNode = isReject && (
      <Reason>{data.reason}</Reason>
    )
    return (
      <RejectRow key={key}>
        {userNode}
        {reasonNode}
      </RejectRow>
    )
  })
  return (
    <Container>
      <Label>{label}</Label>
      <List type={type}>{votesNode}</List>
    </Container>
  )
}

const propTypes = {
  label: PropTypes.string.isRequired,
  type: PropTypes.string.isRequired,
  dataList: PropTypes.array.isRequired,
}

Component.propTypes = propTypes


export default Component
