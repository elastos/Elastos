import React from 'react'
import { Row, Col, Spin, Button } from 'antd'
import styled from 'styled-components'
import I18N from '@/I18N'
import StandardPage from '@/module/page/StandardPage'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import ElipNote from '../ElipNote'
import { grid } from '../common/variable'

class C extends StandardPage {
  ord_renderContent() {
    const { isSecretary } = this.props
    return (
      <div>
        <BackLink link="/elips" />
        <Container>
          <h2 className="komu-a cr-title-with-icon">ELIP #100</h2>
          <Label>status</Label>
          <Status>WAIT FOR REVIEW</Status>
          <Row>
            <LabelCol span={3}>{I18N.get('elip.fields.title')}</LabelCol>
            <WrapperCol span={17}>
              <Title>hello elip</Title>
            </WrapperCol>
          </Row>
          <ElipNote />
          <Row>
            <LabelCol span={3}>{I18N.get('elip.fields.description')}</LabelCol>
            <WrapperCol span={17}>
              <Dec>hello elip</Dec>
            </WrapperCol>
          </Row>
          <Row>
            <LabelCol span={3} />
            <Col span={17}>
              <Actions>
                <Button
                  onClick={() => this.props.history.push('/elips')}
                  className="cr-btn cr-btn-default"
                  style={{ marginRight: 10 }}
                >
                  {I18N.get('elip.button.cancel')}
                </Button>
                {isSecretary && (
                  <Button
                    className="cr-btn cr-btn-danger"
                    style={{ marginRight: 10 }}
                  >
                    {I18N.get('elip.button.reject')}
                  </Button>
                )}
                {isSecretary && (
                  <Button className="cr-btn cr-btn-primary">
                    {I18N.get('elip.button.approve')}
                  </Button>
                )}
              </Actions>
            </Col>
          </Row>
        </Container>
        <Footer />
      </div>
    )
  }
}

export default C

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

const Label = styled.div`
  font-size: 11px;
  line-height: 19px;
  color: rgba(3, 30, 40, 0.4);
`

const Status = styled.div`
  font-size: 16px;
  line-height: 27px;
  text-transform: uppercase;
  color: #000000;
  margin-bottom: 42px;
  background: #F2F6FB;
  width: 159px;
  height: 27px;
  text-align: center;
`

const LabelCol = styled(Col)`
  min-width: 120px;
  text-align: right;
  font-size: 18px;
  margin-right: 20px;
`

const WrapperCol = styled(Col)`
  border: 1px solid rgba(0, 0, 0, 0.1);
  background: rgba(204, 204, 204, 0.2);
`

const Title = styled.div`
  padding: 16px;
  color: #000;
`

const Dec = styled.div`
  height: 320px;
  padding: 16px;
  font-size: 14px;
  line-height: 20px;
  color: #000;
`

const Actions = styled.div`
  margin-top: 60px;
  display: flex;
  justify-content: center;
`
