import React from 'react'
import styled from 'styled-components'
import Footer from '@/module/layout/Footer/Container'
import StandardPage from '@/module/page/StandardPage'
import ElipForm from '@/module/form/ElipForm/Container'
import BackLink from '@/module/shared/BackLink/Component'
import { grid } from '../common/variable'

export default class extends StandardPage {
  constructor(p) {
    super(p)
    this.state = {
      loading: false
    }
  }

  ord_renderContent() {
    return (
      <div>
        <Container>
          <BackLink link='/elips' />
          <ElipForm />
        </Container>
        <Footer />
      </div>
    )
  }
}

const Container = styled.div`
  padding: 0 50px 80px;
  width: 70vw;
  margin: 80px auto 0;
  background: #ffffff;
  text-align: left;
  @media only screen and (max-width: ${grid.sm}) {
    margin: 15px;
  }
`
