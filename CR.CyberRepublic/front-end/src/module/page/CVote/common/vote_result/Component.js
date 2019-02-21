import React from 'react'
import PropTypes from 'prop-types'
import _ from 'lodash'

import './style.scss'

const Component = ({ label, type, dataList }) => {
  const votesNode = _.map(dataList, (data, key) => {
    return (
      <div className="voter-item" key={key}>
        <img src={data.avatar} alt="voter avatar" />
        <div>{data.name}</div>
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
  dataList: PropTypes.array.isRequired,
}

Component.propTypes = propTypes


export default Component
