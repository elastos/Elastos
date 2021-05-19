import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import styled from 'styled-components'

import './style.scss'

export default class extends BaseComponent {
  ord_render() {
    return (
      <Container>
        <div className="circle-container">
          <img className="circle" src="/assets/images/council_circle.png" />
        </div>
        <div className="circle-top1">
          <img className="circle" src="/assets/images/council_circle.png" />
        </div>
        <div className="circle-top2">
          <img className="circle" src="/assets/images/council_circle.png" />
        </div>
        <div className="right-box-container">
          <div className="small-box" />
          <div className="box" />
          <img src="/assets/images/training_green_slashed_box.png" />
        </div>
        <div className="connector-container">
          <img src="/assets/images/council_connector.png" />
        </div>
      </Container>
    )
  }
}

const Container = styled.div`
`
