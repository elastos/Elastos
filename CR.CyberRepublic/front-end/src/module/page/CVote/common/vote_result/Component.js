import React from 'react'
import PropTypes from 'prop-types'
import _ from 'lodash'
import { Row, Col } from 'antd'

import './style.scss'

const Component = ({ label, type, dataList }) => {
  const votesNode = _.map(dataList, (data, key) => {
    const span = data.reason ? 24 : 6
    const innerSpan = data.reason ? 6 : 24
    return (
      <Col className="voter-item" key={key} span={span}>
        <Row>
          <Col span={innerSpan}>
            <img src={data.avatar} alt="voter avatar" />
            <div>{data.name}</div>
          </Col>
          <Col span={24 - innerSpan}>
            <div className="reason">{data.reason}</div>
          </Col>
        </Row>
      </Col>
    )
  })
  return (
    <div className="c_VoteResult">
      <div className="vote-label">{label}</div>
      <Row className={`voter-list ${type}`}>{votesNode}</Row>
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
