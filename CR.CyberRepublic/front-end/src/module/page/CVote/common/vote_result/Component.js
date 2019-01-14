import React from 'react'
import PropTypes from 'prop-types'
import _ from 'lodash'

import './style.scss'

const Component = ({ label, type, users }) => {
  const votesNode = _.map(users, (user, key) => {
    return (
      <div className="voter-item" key={key}>
        <img src={user.avatar} alt="voter avatar" />
        <div>{user.name}</div>
      </div>
    )
  })
  return (
    <div className="c_VoteResult">
      <div className="vote-label">{label}</div>
      <div className={`voter-list ${type}`}>{votesNode}</div>
    </div>
  )
}

const propTypes = {
  label: PropTypes.string.isRequired,
  type: PropTypes.string.isRequired,
  users: PropTypes.array.isRequired,
}

Component.propTypes = propTypes


export default Component
