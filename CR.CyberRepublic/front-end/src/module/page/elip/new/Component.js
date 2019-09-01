import React from 'react'
import styled from 'styled-components'
import Footer from '@/module/layout/Footer/Container'
import StandardPage from '@/module/page/StandardPage'
import ElipForm from '@/module/form/ElipForm/Container'
import BackLink from '@/module/shared/BackLink/Component'
import { breakPoint } from '@/constants/breakPoint'

export default class extends StandardPage {
  ord_renderContent() {
    const { isLogin, history } = this.props
    if (!isLogin) {
      history.push('/login')
    }
    return (
      <Wrapper>
        <BackLink link='/elips' />
        <Container>
          <ElipForm />
        </Container>
        <Footer />
      </Wrapper>
    )
  }
}

const Wrapper = styled.div`
  margin-top: 64px;
  position: relative;
  .cr-backlink {
    top: -32px;
    left: 16px;
  }
`

const Container = styled.div`
  padding: 0 50px 80px;
  width: 70vw;
  margin: 80px auto 0;
  background: #ffffff;
  text-align: left;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-top: 48px;
    padding: 0;
    width: 100%;
  }
`
