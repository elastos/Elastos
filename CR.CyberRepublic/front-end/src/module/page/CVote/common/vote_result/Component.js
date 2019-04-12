import React from 'react'
import PropTypes from 'prop-types'
import _ from 'lodash'

import { Container, ResultRow, Reason, Label, List, Item, Avatar } from './style'

const Component = ({ label, type, dataList }) => {
  const votesNode = _.map(dataList, (data, key) => {
    // const isReject = type === CVOTE_RESULT.REJECT
    const userNode = (
      <Item key={key}>
        <Avatar src={data.avatar} alt="voter avatar" />
        <div>{data.name}</div>
      </Item>
    )

    // if (!isReject) return userNode
    // show reason for all vote type
    const reasonNode = (
      <Reason>{data.reason}</Reason>
    )
    return (
      <ResultRow key={key}>
        {userNode}
        {reasonNode}
      </ResultRow>
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
