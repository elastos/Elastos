import React from 'react'
import styled from 'styled-components'
import Footer from '@/module/layout/Footer/Container'
import StandardPage from '@/module/page/StandardPage'
import ElipForm from '@/module/form/ElipForm/Container'
import BackLink from '@/module/shared/BackLink/Component'
import { breakPoint } from '@/constants/breakPoint'

export default class extends StandardPage {
  constructor(props) {
    super(props)

    this.state = {
      error: null
    }
  }

  historyBack = () => {
    this.props.history.push('/elips')
  }

  onSubmit = model => {
    return this.props
      .createElip(model)
      .then(() => this.historyBack())
      .catch(err => this.setState({ error: err }))
  }

  ord_renderContent() {
    const { isLogin, history } = this.props
    if (!isLogin) {
      history.push('/login')
    }
    return (
      <Wrapper>
        <BackLink link="/elips" />
        <Container>
          <ElipForm onSubmit={this.onSubmit} onCancel={this.historyBack} />
        </Container>
        <Footer />
      </Wrapper>
    )
  }
}

const Wrapper = styled.div`
  margin-top: 64px;
  position: relative;
`

const Container = styled.div`
  padding: 0 50px 80px;
  width: 80vw;
  margin: 80px auto 0;
  background: #ffffff;
  text-align: left;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-top: 48px;
    padding: 0;
    width: 100%;
  }
`
