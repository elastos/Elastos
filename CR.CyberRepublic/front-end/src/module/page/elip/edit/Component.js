import React from 'react'
import { Spin } from 'antd'
import styled from 'styled-components'
import Footer from '@/module/layout/Footer/Container'
import StandardPage from '@/module/page/StandardPage'
import ElipForm from '@/module/form/ElipForm/Container'
import BackLink from '@/module/shared/BackLink/Component'
import { ELIP_STATUS } from '@/constant'
import { grid } from '../common/variable'

export default class extends StandardPage {
  constructor(p) {
    super(p)
    this.state = {
      loading: true,
      elip: {}
    }
  }

  async componentDidMount() {
    const { getData, match } = this.props
    const data = await getData(match.params)
    this.setState({ elip: data.elip, loading: false })
  }

  ord_renderContent() {
    const { isLogin, currentUserId, history } = this.props
    if (!isLogin) {
      return history.push('/login')
    }
    const { elip, loading } = this.state
    if (loading) {
      return <StyledSpin><Spin /></StyledSpin>
    }
    if (!loading && !Object.keys(elip).length) {
      return history.push('/elips')
    }
    const isVisible =
      !loading &&
      elip.status === ELIP_STATUS.REJECTED &&
      elip.createdBy._id === currentUserId
    if (!isVisible) {
      return history.push(`/elips/${elip._id}`)
    }

    return (
      <div>
        <Container>
          <BackLink link="/elips" />
          <ElipForm data={elip} />
        </Container>
        <Footer />
      </div>
    )
  }
}

const StyledSpin = styled.div`
  text-align: center;
  margin-top: 24px;
`

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
