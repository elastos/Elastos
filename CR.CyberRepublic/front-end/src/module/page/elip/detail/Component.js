import React from 'react'
import { Row, Col, Spin, Button, message, Popconfirm, Anchor } from 'antd'
import styled from 'styled-components'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import I18N from '@/I18N'
import _ from 'lodash'
import StandardPage from '@/module/page/StandardPage'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import Meta from '@/module/common/Meta'
import MetaComponent from '@/module/shared/meta/Container'
import { ELIP_STATUS } from '@/constant'
import { text } from '@/constants/color'
import { logger } from '@/util'
import { breakPoint } from '@/constants/breakPoint'
import moment from 'moment/moment'
import ReviewButtons from './ReviewButtons'
import ReviewHistory from './ReviewHistory'

const { Link } = Anchor

class C extends StandardPage {
  constructor(p) {
    super(p)

    this.state = {
      loading: false
    }
  }

  async componentDidMount() {
    const { getData, match } = this.props
    try {
      await getData(match.params)
    } catch (err) {
      logger.error(err)
    }
  }

  componentWillUnmount() {
    this.props.resetData()
  }

  handleSubmit = async e => {
    const { review, data } = this.props
    const param = {
      comment: e.reason,
      status: e.status,
      elipId: data._id
    }
    try {
      const comment = await review(param)
      if (comment && comment.elipId === data._id) {
        this.setState({
          elip: { ...data, status: comment.elipStatus },
          reviews: [...this.state.reviews, comment]
        })
        if (comment.elipStatus === ELIP_STATUS.DRAFT) {
          message.info(I18N.get('elip.msg.approved'))
        }
        if (comment.elipStatus === ELIP_STATUS.REJECTED) {
          message.info(I18N.get('elip.msg.rejected'))
        }
      }
    } catch (err) {
      logger.error(err)
    }
  }

  ord_renderContent() {
    const { data, loading } = this.props
    if (loading) {
      return (
        <StyledSpin>
          <Spin />
        </StyledSpin>
      )
    }

    return (
      <Wrapper>
        <Meta
          title={`${data.title} - ELIP Detail - Cyber Republic`}
          url={this.props.location.pathname}
        />
        <Container>
          <BackLink link="/elips" />
          {this.renderAnchors()}
          {this.renderDetail()}
        </Container>
        <Footer />
      </Wrapper>
    )
  }

  isAuthor(elip) {
    const { currentUserId } = this.props
    return elip.createdBy && elip.createdBy._id === currentUserId
  }

  submittedDraft = async () => {
    await this.updateStatus(ELIP_STATUS.SUBMITTED)
  }

  cancelledSubmitted = async () => {
    await this.updateStatus(ELIP_STATUS.CANCELLED)
  }

  updateStatus = async (elipStatus) => {
    try {
      const { updateStatus, data } = this.props
      const rs = await updateStatus({
        _id: data._id,
        status: elipStatus
      })
      if (rs && rs.ok === 1 && rs.n === 1) {
        this.setState({
          data: { ...data, status: elipStatus }
        })
        message.info(I18N.get('elip.msg.marked'))
      }
    } catch (error) {
      logger.error(error)
    }
  }

  renderAnchors() {
    const { data, isSecretary } = this.props
    const reviewLink =
      isSecretary || this.isAuthor(data) ? (
        <LinkGroup marginTop={30}>
          <Link href="#review" title={I18N.get('elip.fields.review')} />
        </LinkGroup>
      ) : (
        ''
      )
    return (
      <StyledAnchor offsetTop={420}>
        <LinkGroup>
          <Link href="#preamble" title={I18N.get('elip.fields.preamble')} />
          <Link href="#abstract" title={I18N.get('elip.fields.abstract')} />
        </LinkGroup>
        <LinkGroup marginTop={48}>
          <Link
            href="#specifications"
            title={I18N.get('elip.fields.specifications')}
          />
          <Link href="#motivation" title={I18N.get('elip.fields.motivation')} />
          <Link href="#rationale" title={I18N.get('elip.fields.rationale')} />
        </LinkGroup>
        <LinkGroup marginTop={46}>
          <Link
            href="#backwardCompatibility"
            title={I18N.get('elip.fields.backwardCompatibility')}
          />
          <Link
            href="#referenceImplementation"
            title={I18N.get('elip.fields.referenceImplementation')}
          />
          <Link
            href="#copyrightDomain"
            title={I18N.get('elip.fields.copyrightDomain')}
          />
        </LinkGroup>
        {reviewLink}
      </StyledAnchor>
    )
  }

  renderDetail() {
    const { data } = this.props
    const sections = [
      'abstract',
      'specifications',
      'motivation',
      'rationale',
      'backwardCompatibility',
      'referenceImplementation',
      'copyrightDomain'
    ]

    const metaNode = this.renderMeta()
    const titleNode = this.renderTitleNode()
    const subTitleNode = this.renderSubTitle()
    const preamble = this.renderPreamble()
    const review = this.renderReview()

    return (
      <Content>
        {metaNode}
        {titleNode}
        {subTitleNode}
        {preamble}
        {_.map(sections, section => (
          <Part id={section} key={section}>
            <PartTitle>{I18N.get(`elip.fields.${section}`)}</PartTitle>
            <PartContent>
              <MarkdownPreview content={data[section] ? data[section] : ''} />
            </PartContent>
          </Part>
        ))}
        {review}
      </Content>
    )
  }

  renderMeta() {
    const { data } = this.props
    data.proposer = data.createdBy
    data.displayId = data.vid
    const postedByText = I18N.get('from.CVoteForm.label.proposedby')
    return <MetaComponent data={data} postedByText={postedByText} />
  }

  renderTitleNode() {
    const { data } = this.props
    return <Title>{data.title}</Title>
  }

  renderSubTitle() {
    const status = this.renderStatus()
    const edit = this.renderEditButton()
    const submitted = this.renderSubmittedButton()
    const cancelled = this.renderCancelledButton()
    return (
      <Row type="flex" justify="start" gutter={25.5}>
        {status}
        {edit}
        {submitted}
        {cancelled}
      </Row>
    )
  }

  renderPreamble() {
    const { data } = this.props
    const preambles = {
      elip: `#${data.vid}`,
      title: data.title,
      author: data.createdBy && data.createdBy.username,
      discussions: data.discussions,
      status: data.status,
      type: data.type,
      created: moment(data.createdAt).format('MMM D, YYYY'),
      requires: data.requires,
      replaces: data.replaces,
      superseded: data.superseded
    }

    return (
      <Part id="preamble">
        <PartTitle>{I18N.get('elip.fields.preamble')}</PartTitle>
        <PartContent className="preamble">
          {_.map(preambles, (v, k) => this.renderPreambleItem(I18N.get(`elip.fields.preambleItems.${k}`), v))}
        </PartContent>
      </Part>
    )
  }

  renderPreambleItem(key, value) {
    return (
      <Row key={key}>
        <Col span={6}>
          <PartContent>{`${key}:`}</PartContent>
        </Col>
        <Col span={18}>
          <PartContent>{value}</PartContent>
        </Col>
      </Row>
    )
  }

  renderReview() {
    const review = this.renderReviewButton()
    const reviewHistory = this.renderReviewHistory()

    return (
      <Part id="review">
        {review}
        {reviewHistory}
      </Part>
    )
  }

  renderStatus() {
    const { data } = this.props
    return (
      <Col>
        <Label>{I18N.get('.status')}</Label>
        <Status status={data.status}>
          {I18N.get(`elip.status.${data.status}`)}
        </Status>
      </Col>
    )
  }

  renderEditButton() {
    const { data } = this.props
    const isEditable =
      this.isAuthor(data) &&
      [ELIP_STATUS.REJECTED, ELIP_STATUS.DRAFT].includes(data.status)

    if (isEditable) {
      const btnStyle =
        ELIP_STATUS.DRAFT === data.status ? 'cr-btn-black' : 'cr-btn-primary'
      return (
        <Col>
          <Button
            onClick={() => this.props.history.push(`/elips/${data._id}/edit`)}
            className={`cr-btn ${btnStyle}`}
          >
            {I18N.get('elip.button.edit')}
          </Button>
        </Col>
      )
    }
    return ''
  }

  renderSubmittedButton() {
    const { data } = this.props
    const isEditable = this.isAuthor(data) && data.status === ELIP_STATUS.DRAFT
    if (isEditable) {
      return (
        <Col>
          <Popconfirm
            title={I18N.get('elip.modal.markAsSubmitted')}
            onConfirm={this.submittedDraft}
            okText={I18N.get('.yes')}
            cancelText={I18N.get('.no')}
          >
            <Button className="cr-btn cr-btn-primary">
              {I18N.get('elip.button.markAsSubmitted')}
            </Button>
          </Popconfirm>
        </Col>
      )
    }
    return ''
  }

  renderCancelledButton() {
    const { data, isCouncil } = this.props
    const canCancel = isCouncil && ELIP_STATUS.SUBMITTED === data.status
    if (canCancel) {
      return (
        <Col>
          <Popconfirm
            title={I18N.get('elip.modal.markAsCancelled')}
            onConfirm={this.cancelledSubmitted}
            okText={I18N.get('.yes')}
            cancelText={I18N.get('.no')}
          >
            <Button className="cr-btn cr-btn-black">
              {I18N.get('elip.button.cancel')}
            </Button>
          </Popconfirm>
        </Col>
      )
    }
    return ''
  }

  renderReviewButton() {
    const { data, isSecretary } = this.props
    const isVisible = isSecretary && data.status === ELIP_STATUS.WAIT_FOR_REVIEW
    if (isVisible) {
      return (
        <Row>
          <LabelCol span={3} />
          <Col span={17}>
            <Actions>
              <ReviewButtons onSubmit={this.handleSubmit} />
            </Actions>
          </Col>
        </Row>
      )
    }
    return ''
  }

  renderReviewHistory() {
    const { data, reviews, isSecretary } = this.props
    if (this.isAuthor(data) || isSecretary) {
      return (
        <Row>
          <LabelCol span={3} />
          <Col span={17} style={{ position: 'relative' }}>
            <ReviewHistory reviews={reviews} />
          </Col>
        </Row>
      )
    }
    return ''
  }
}

export default C

const StyledSpin = styled.div`
  text-align: center;
  margin-top: 24px;
`

const Wrapper = styled.div`
  position: relative;
`

const Container = styled.div`
  padding: 30px 50px 80px 150px;
  width: 80vw;
  margin: 0 auto;
  background: #ffffff;
  text-align: left;
  .cr-backlink {
    position: fixed;
    left: 27px;
  }
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-top: 48px;
    padding: 16px;
    width: 100%;
    .cr-backlink {
      display: none;
    }
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
  font-family: Synthese;
  font-size: 16px;
  line-height: 23px;
  text-transform: uppercase;
  color: ${props => {
    return props.status === ELIP_STATUS.REJECTED ? '#fff' : '#000'
  }};
  margin-bottom: 42px;
  background: ${props => {
    switch (props.status) {
      case ELIP_STATUS.REJECTED:
        return '#BE1313'
      default:
        return '#D6D6D6'
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

const Title = styled.h2`
  font-family: Synthese;
  font-size: 30px;
  line-height: 42px;
  color: #031e28;
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

const StyledAnchor = styled(Anchor)`
  position: fixed;
  top: 250px;
  left: 30px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    display: none;
  }
  .ant-anchor-ink:before {
    width: 0;
  }
  .ant-anchor-ink-ball.visible {
    display: none;
  }
  .ant-anchor-link-title {
    display: inline;
  }
  .ant-anchor-link-active > .ant-anchor-link-title {
    color: initial;
    :after {
      content: '';
      position: absolute;
      bottom: -2px;
      left: 0;
      right: 0;
      height: 0.5em;
      border-bottom: 8px solid ${text.green};
      z-index: -1;
    }
  }
`

const LinkGroup = styled.div`
  ${props => props.marginTop && `margin-top: ${props.marginTop}px;`}
`

const Content = styled.article``

const Part = styled.div`
  .preamble {
    font-family: Synthese;
    font-size: 13px;
    line-height: 18px;
    align-items: center;
    color: #000;
  }
`

const PartTitle = styled.h4`
  font-family: Synthese;
  font-size: 20px;
  line-height: 28px;
  color: #000;
`

const PartContent = styled.div``
