import React from 'react'
import { Row, Col, Spin, Button } from 'antd'
import styled from 'styled-components'
import DraftEditor from '@/module/common/DraftEditor'
import I18N from '@/I18N'
import StandardPage from '@/module/page/StandardPage'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
// import Comments from '@/module/common/comments/Container'
import { ELIP_STATUS } from '@/constant'
import { logger } from '@/util'
import ElipNote from '../ElipNote'
import { grid } from '../common/variable'
import ReviewButtons from './ReviewButtons'
import ReviewHistory from './ReviewHistory'

class C extends StandardPage {
  constructor(p) {
    super(p)
    this.state = {
      elip: {},
      loading: false,
      reviews: []
    }
  }

  async componentDidMount() {
    const { getData, match } = this.props
    this.setState({ loading: true })
    try {
      const data = await getData(match.params)
      this.setState({
        elip: data.elip,
        loading: false,
        reviews: data.reviews
      })
    } catch (err) {
      logger.error(err)
    }
  }

  handleSubmit = async data => {
    const { review } = this.props
    const { elip } = this.state
    const param = {
      comment: data.reason,
      status: data.status,
      elipId: elip._id
    }
    try {
      const comment = await review(param)
      if (comment && comment.elipId === elip._id) {
        this.setState({
          elip: { ...elip, status: data.status },
          reviews: [...this.state.reviews, comment]
        })
      }
    } catch (err) {
      logger.error(err)
    }
  }

  ord_renderContent() {
    const { elip, loading, reviews } = this.state
    if (loading) {
      return <Spin />
    }
    if (!loading && !Object.keys(elip).length) {
      return null
    }
    const { isLogin, isSecretary } = this.props
    return (
      <div>
        <BackLink link="/elips" />
        <Container>
          <h2 className="komu-a cr-title-with-icon">ELIP #{elip.vid}</h2>
          <Label>Status</Label>
          <Status status={elip.status}>{elip.status}</Status>
          <Row>
            <LabelCol span={3}>{I18N.get('elip.fields.title')}</LabelCol>
            <WrapperCol span={17}>
              <Title>{elip.title}</Title>
            </WrapperCol>
          </Row>
          <ElipNote />
          <Row>
            <LabelCol span={3}>{I18N.get('elip.fields.description')}</LabelCol>
            <WrapperCol span={17}>
              <Dec status={elip.status}>
                <StyledRichContent>
                  <DraftEditor
                    value={elip.description}
                    contentType={elip.contentType}
                    editorEnabled={false}
                  />
                </StyledRichContent>
              </Dec>
            </WrapperCol>
          </Row>
          {elip && elip.status !== ELIP_STATUS.APPROVED && (
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
                  {this.renderEditBtn()}
                  {isLogin &&
                    isSecretary &&
                    elip.status === ELIP_STATUS.WAIT_FOR_REVIEW && (
                      <ReviewButtons onSubmit={this.handleSubmit} />
                    )}
                </Actions>
              </Col>
            </Row>
          )}
          {elip && elip.status !== ELIP_STATUS.APPROVED && (
            <ReviewHistory reviews={reviews} />
          )}
        </Container>
        <Footer />
      </div>
    )
  }

  renderEditBtn() {
    const { elip } = this.state
    const { isLogin, currentUserId } = this.props

    const isEditable = isLogin &&
      elip.createdBy &&
      elip.createdBy._id === currentUserId &&
      elip.status === ELIP_STATUS.REJECTED

    if (isEditable) {
      return (
        <Button
          onClick={() => this.props.history.push(`/elips/${elip._id}/edit`)}
          className="cr-btn cr-btn-primary"
        >
          {I18N.get('elip.button.edit')}
        </Button>
      )
    }
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
  color: ${props => {
    return props.status === ELIP_STATUS.WAIT_FOR_REVIEW ? '#000' : '#fff'
  }};
  margin-bottom: 42px;
  background: ${props => {
    switch (props.status) {
      case ELIP_STATUS.REJECTED:
        return '#be1313'
      case ELIP_STATUS.APPROVED:
        return '#008d85'
      default:
        return '#f2f6fb'
    }
  }};
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
  padding: 24px;
  color: #000;
`

const Dec = styled.div`
  min-height: 320px;
  padding: 24px;
  font-size: 14px;
  line-height: 20px;
  color: #000;
  background: ${props => {
    switch (props.status) {
      case ELIP_STATUS.REJECTED:
        return 'rgba(252, 192, 192, 0.2)'
      case ELIP_STATUS.APPROVED:
        return 'rgba(29, 233, 182, 0.1)'
      default:
        return 'background: rgba(204, 204, 204, 0.2)'
    }
  }};
`

const Actions = styled.div`
  margin-top: 60px;
  display: flex;
  justify-content: center;
`

export const StyledRichContent = styled.div`
  .md-RichEditor-root {
    background: none;
    padding: 0;
    .public-DraftEditor-content {
      padding: 0;
      margin: 0;
    }
    figure.md-block-image {
      background: none;
    }
    figure.md-block-image figcaption .public-DraftStyleDefault-block {
      text-align: left;
    }
  }
`
