import React from 'react'
import BaseComponent from '@/model/BaseComponent'

import I18N from '@/I18N'
import { Col, Avatar } from 'antd'
import styled from 'styled-components'

import './style.scss'

export default class extends BaseComponent {
  ord_render() {
    const { title, desc, link } = this.props
    return (
      <Col lg={8} md={8} sm={24} className="member">
        <div className="small-rect">
          <Avatar shape="square" size={220} icon="user" />
        </div>

        <div className="big-rect">
          <div className="content">
            <h3 className="name">{title}</h3>
            <span className="self-intro">{desc}</span>
            <div><ViewMore href={link}>{I18N.get('cs.secretariat.positions.viewMore')}</ViewMore></div>
          </div>
        </div>
      </Col>
    )
  }
}

const ViewMore = styled.a`
  color: white;
  background-image
`