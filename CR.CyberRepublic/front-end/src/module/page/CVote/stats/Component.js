import React from 'react'
import PropTypes from 'prop-types'
import _ from 'lodash'

import './style.scss'

const Component = ({ percentage, values, yes }) => {
  const statusGroup = _.map(values, (value, key) => {
    const statusClass = `vote-status-item ${value.toLowerCase()}` // yes, no, undecided
    return <div className={statusClass} key={key} />
  })
  const textClass = `vote-status-text ${yes ? 'agreed' : ''}`
  return (
    <div className="c_VoteStatus">
      <div className={textClass}>{percentage}</div>
      <div className="vote-status-list">{statusGroup}</div>
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
