import React from 'react'
import { Row, Col, Spin, Button, message } from 'antd'
import styled from 'styled-components'
import DraftEditor from '@/module/common/DraftEditor'
import I18N from '@/I18N'
import StandardPage from '@/module/page/StandardPage'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import Comments from '@/module/common/comments/Container'
import { ELIP_STATUS } from '@/constant'
import { logger } from '@/util'
import Meta from '@/module/common/Meta'
import { breakPoint } from '@/constants/breakPoint'
import ElipNote from '../ElipNote'
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

        if (data.status === ELIP_STATUS.DRAFT) {
          message.info(I18N.get(`elip.msg.approved`))
        } else {
          message.info(I18N.get(`elip.msg.rejected`))
        }
      }
    } catch (err) {
      logger.error(err)
    }
  }

  ord_renderContent() {
    const { elip, loading } = this.state
    if (loading) {
      return (
        <StyledSpin>
          <Spin />
        </StyledSpin>
      )
    }
    if (!loading && !Object.keys(elip).length) {
      return null
    }

    return (
      <Wrapper>
        <Meta
          title={`${elip.title} - ELIP Detail - Cyber Republic`}
          url={this.props.location.pathname}
        />
        <BackLink link="/elips" />
        <Container>
          <h2 className="komu-a cr-title-with-icon">ELIP #{elip.vid}</h2>
          <Label>Status</Label>
          <Status status={elip.status}>
            {elip.status.split('_').join(' ')}
          </Status>
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
          {this.renderEditButton()}
          {this.renderReviewButtons()}
          {this.renderReviewHistory()}
          {elip.status === ELIP_STATUS.DRAFT && (
            <Row style={{ marginTop: 24 }}>
              <LabelCol span={3} />
              <Col span={17}>
                <Comments
                  type="elip"
                  elip={elip}
                  canPost={true}
                  model={elip._id}
                  returnUrl={`/elips/${elip._id}`}
                />
              </Col>
            </Row>
          )}
        </Container>
        <Footer />
      </Wrapper>
    )
  }

  isAuthor(elip) {
    const { currentUserId } = this.props
    return elip.createdBy && elip.createdBy._id === currentUserId
  }

  renderEditButton() {
    const { elip } = this.state
    const isEditable = this.isAuthor(elip) && elip.status === ELIP_STATUS.REJECTED
    if (isEditable) {
      return (
        <Row>
          <LabelCol span={3} />
          <Col span={17}>
            <Actions>
              <Button
                onClick={() => this.props.history.push(`/elips/${elip._id}/edit`)}
                className="cr-btn cr-btn-primary"
              >
                {I18N.get('elip.button.edit')}
              </Button>
            </Actions>
          </Col>
        </Row>
      )
    }
  }

  renderReviewButtons() {
    const { elip } = this.state
    const { isSecretary, history } = this.props
    const isVisible = isSecretary && elip.status === ELIP_STATUS.WAIT_FOR_REVIEW
    if (isVisible) {
      return (
        <Row>
          <LabelCol span={3} />
          <Col span={17}>
            <Actions>
              <Button
                onClick={() => history.push('/elips')}
                className="cr-btn cr-btn-default"
                style={{ marginRight: 10 }}
              >
                {I18N.get('elip.button.cancel')}
              </Button>
              <ReviewButtons onSubmit={this.handleSubmit} />
            </Actions>
          </Col>
        </Row>
      )
    }
  }

  renderReviewHistory() {
    const { reviews, elip } = this.state
    const { isSecretary } = this.props
    if (this.isAuthor(elip) || isSecretary) {
      return (
        <Row>
          <LabelCol span={3} />
          <Col span={17} style={{ position: 'relative' }}>
            <ReviewHistory reviews={reviews} />
          </Col>
        </Row>
      )
    }
  }
}

export default C

const StyledSpin = styled.div`
  text-align: center;
  margin-top: 24px;
`

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
    padding: 16px;
    width: 100%;
  }
  .ant-row .ant-col-17 {
    @media only screen and (max-width: ${breakPoint.mobile}) {
      width: 100%;
    }
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
      case ELIP_STATUS.DRAFT:
        return '#008d85'
      default:
        return '#f2f6fb'
    }
  }};
  text-align: center;
  padding: 0 6px;
  display: inline-block;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-bottom: 32px;
    font-size: 14px;
  }
`

const LabelCol = styled(Col)`
  min-width: 120px;
  text-align: right;
  font-size: 18px;
  margin-right: 20px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    padding-bottom: 8px;
    text-align: left;
  }
`

const WrapperCol = styled(Col)`
  border: 1px solid rgba(0, 0, 0, 0.1);
  background: rgba(204, 204, 204, 0.2);
`

const Title = styled.div`
  padding: 8px 20px;
  color: #000;
  line-height: 1.8;
`

const Dec = styled.div`
  min-height: 320px;
  padding: 20px;
  font-size: 14px;
  line-height: 20px;
  color: #000;
  background: ${props => {
    switch (props.status) {
      case ELIP_STATUS.REJECTED:
        return 'rgba(252, 192, 192, 0.2)'
      case ELIP_STATUS.DRAFT:
        return 'rgba(29, 233, 182, 0.1)'
      default:
        return 'background: rgba(204, 204, 204, 0.2)'
    }
  }};
`

const Actions = styled.div`
  margin-top: 52px;
  display: flex;
  justify-content: center;
  flex-wrap: wrap;
  button {
    margin: 8px 0;
  }
`

export const StyledRichContent = styled.div`
  .md-RichEditor-root {
    background: none;
    padding: 0;
    .md-RichEditor-editor {
      .md-RichEditor-blockquote {
        border-left: 5px solid #ccc;
        background-color: unset;
        font-size: 1.1em;
      }
      .public-DraftEditor-content {
        padding: 0;
        margin: 0;
      }
    }
    figure.md-block-image {
      background: none;
      figcaption .public-DraftStyleDefault-block {
        text-align: left;
      }
    }
  }
`
