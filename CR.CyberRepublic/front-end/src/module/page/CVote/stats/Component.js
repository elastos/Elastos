import React from 'react'
import PropTypes from 'prop-types'
import _ from 'lodash'
import { CVOTE_RESULT } from '@/constant'

import { List, Item, ItemUndecided, Text } from './style'

const Component = ({ percentage, values, yes }) => {
  const statusGroup = _.map(values, (status, key) => {
    if (status === CVOTE_RESULT.UNDECIDED) {
      return <ItemUndecided status={status} key={key} />
    }
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
  yes: PropTypes.bool.isRequired
}

Component.propTypes = propTypes

export default Component
