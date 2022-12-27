import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Spin } from 'antd'
import styled from 'styled-components'

export default class extends BaseComponent {

  ord_render() {
    return (
      <Container>
        <Spin className="cr-loading-item" size="large" delay={100} />
      </Container>
    )
  }
}

const Container = styled.div`
  position: relative;
  width: 100vw;
  height: 100vh;
  .cr-loading-item {
    position: relative;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
  }
`
