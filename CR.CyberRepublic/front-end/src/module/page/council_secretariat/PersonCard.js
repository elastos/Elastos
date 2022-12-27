import React from 'react'
import BaseComponent from '@/model/BaseComponent'

import I18N from '@/I18N'
import { Col, Avatar } from 'antd'
import styled from 'styled-components'
import { Link } from 'react-router-dom'

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
            <Desc>
              <Title className="name">{title}</Title>
              <Intro>{desc}</Intro>
            </Desc>
            <ViewMore to={link}>
              {I18N.get('cs.secretariat.positions.viewMore')}
              <Arrow src="/assets/images/arrow.svg" />
            </ViewMore>
          </div>
        </div>
      </Col>
    )
  }
}

const Desc = styled.div`
  min-height: 120px;
`

const Title = styled.h3`
  font-size: 28px;
  color: #FFFFFF;
`

const Intro = styled.span`
  font-size: 13px;
  color: #F6F9FD;
  opacity: 0.9;
`

const ViewMore = styled(Link)`
  display: block;
  margin-top: 15px;
  color: white;
`

const Arrow = styled.img`
  width: 10px;
  margin-left: 10px;
`
