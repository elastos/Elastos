import React from 'react'
import PropTypes from 'prop-types'
import _ from 'lodash'

import { List, Item, ItemUndecided, Text } from './style'

const Component = ({ percentage, values, yes }) => {
  const statusGroup = _.map(values, (value, key) => {
    const status = value.toLowerCase() // yes, no, undecided, abstained
    if (status === 'undecided') return <ItemUndecided status={status} key={key} />
    return <Item status={status} key={key} />
  })
  return (
    <div>
      <Text yes={yes}>{percentage}</Text>
      <List>{statusGroup}</List>
    </div>
  )
}

const propTypes = {
  percentage: PropTypes.string.isRequired,
  values: PropTypes.array.isRequired,
  yes: PropTypes.bool.isRequired,
}

Component.propTypes = propTypes


export default Component
