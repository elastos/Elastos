import React from 'react'
import _ from 'lodash'
import {
  Row,
  Col,
  Spin,
  Modal,
  Input,
  Button,
  Anchor,
  Popconfirm,
  message
} from 'antd'
import { Link } from 'react-router-dom'
import MediaQuery from 'react-responsive'
import moment from 'moment/moment'
import Comments from '@/module/common/comments/Container'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import Translation from '@/module/common/Translation/Container'
import SuggestionForm from '@/module/form/SuggestionForm/Container'
import I18N from '@/I18N'
import { LG_WIDTH } from '@/config/constant'
import { CVOTE_STATUS, SUGGESTION_TAG_TYPE } from '@/constant'
import {
  convertMarkdownToHtml,
  removeImageFromMarkdown,
  getPlanHtml,
  getBudgetHtml
} from '@/util/markdown-it'
import { logger } from '@/util'
import URI from 'urijs'
import userUtil from '@/util/user'
import { ReactComponent as CommentIcon } from '@/assets/images/icon-info.svg'
import StandardPage from '../../StandardPage'
import ActionsContainer from '../common/actions/Container'
import MetaContainer from '../common/meta/Container'
import Meta from '@/module/common/Meta'
import SocialShareButtons from '@/module/common/SocialShareButtons'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import TagsContainer from '../common/tags/Container'
import PopoverProfile from '@/module/common/PopoverProfile'
import PaymentList from '@/module/form/SuggestionForm/PaymentList'
import TeamInfoList from '@/module/form/SuggestionForm/TeamInfoList'
import Milestones from '@/module/form/SuggestionForm/Milestones'
import MilestonesReadonly from '@/module/form/SuggestionForm/MilestonesReadonly'
import SignSuggestionButton from './SignSuggetionButton'
import CMSignSuggestionButton from './CMSignSuggestionButton'
import {
  Container,
  Title,
  DescLabel,
  Label,
  LabelPointer,
  BtnGroup,
  StyledButton,
  CouncilComments,
  IconWrap,
  Item,
  ItemTitle,
  ItemText,
  StyledAnchor,
  Subtitle,
  CreateProposalText,
  Paragraph
} from './style'

import './style.scss'
import SignSuggestionModal from './SignSuggestionModal'

const { TextArea } = Input

export default class extends StandardPage {
  constructor(props) {
    super(props)

    // we use the props from the redux store if its retained
    this.state = {
      isDropdownActionOpen: false,
      showMobile: false,
      showForm: false,
      needsInfoVisible: false,
      proposeLoading: false
    }
  }

  async componentDidMount() {
    super.componentDidMount()
    await this.refetch(true)
    await this.props.getDraft(_.get(this.props, 'match.params.id'))
  }

  componentWillUnmount() {
    this.props.resetDetail()
  }

  renderAnchors() {
    const sections = [
      'preamble',
      'abstract',
      'motivation',
      'goal',
      'plan',
      'relevance',
      'budget'
    ]
    return (
      <StyledAnchor offsetTop={420}>
        {sections.map((section) => {
          return (
            <Anchor.Link
              key={section}
              href={`#${section}`}
              title={I18N.get(`suggestion.fields.${section}`)}
            />
          )
        })}
      </StyledAnchor>
    )
  }

  ord_renderContent() {
    const { detail, loading } = this.props
    if (loading || (!loading && _.isEmpty(detail))) {
      return (
        <div className="center">
          <Spin size="large" />
        </div>
      )
    }
    if (detail && detail.success && detail.empty) {
      return (
        <div className="ebp-page">
          <h1>{I18N.get('error.notfound')}</h1>
        </div>
      )
    }
    const detailNode = this.renderDetail(detail)
    const translationBtn = this.renderTranslationBtn()
    const actionsNode = this.renderActionsNode()
    const ownerActionsNode = this.renderOwnerActionsNode()
    const councilActionsNode = this.renderCouncilActionsNode()
    const editForm = this.renderEditForm()
    const commentNode = this.renderCommentNode()
    const socialShareButtonsNode = this.renderSocialShareButtonsNode()

    const uri = URI(this.props.location.search || '')
    const signature = _.get(detail, 'signature.data')

    return (
      <div>
        <Meta
          desc={detail.shortDesc}
          title={`${detail.title} - Suggestion Detail - Cyber Republic`}
          url={this.props.location.pathname}
        />

        <Container className="c_SuggestionDetail">
          <MediaQuery maxWidth={LG_WIDTH}>
            <div>
              <BackLink
                link="/suggestion"
                style={{ position: 'relative', left: 0, marginBottom: 15 }}
              />
              {this.renderAnchors()}
            </div>
            <div>
              {detailNode}
              {translationBtn}
              {socialShareButtonsNode}
              {actionsNode}
              {ownerActionsNode}
              {councilActionsNode}
            </div>
            <div style={{ marginTop: 60 }}>{commentNode}</div>
          </MediaQuery>
          <MediaQuery minWidth={LG_WIDTH + 1}>
            <BackLink
              link="/suggestion"
              style={{ position: 'fixed', left: '27px', top: '189px' }}
            />
            {this.renderAnchors()}
            <Row gutter={24}>
              <Col span={24}>
                {detailNode}
                {translationBtn}
                {socialShareButtonsNode}
                {actionsNode}
                {ownerActionsNode}
                {councilActionsNode}
                <div style={{ marginTop: 60 }}>{commentNode}</div>
              </Col>
            </Row>
          </MediaQuery>
          {editForm}
          {uri.hasQuery('new') && !signature && (
            <SignSuggestionModal
              id={detail._id}
              getSignatureUrl={this.props.getSignatureUrl}
              getSignature={this.props.getSignature}
            />
          )}
        </Container>
        <Footer />
      </div>
    )
  }

  renderPreambleItem(header, value, item) {
    let text = <ItemText>{value}</ItemText>
    const {
      detail: { createdBy },
      user
    } = this.props
    if (item === 'username') {
      text = <PopoverProfile owner={createdBy} curUser={user} />
    }
    return (
      <Item>
        <Col span={6}>
          <ItemTitle>{header}</ItemTitle>
        </Col>
        <Col span={18} style={{ wordBreak: 'break-all' }}>
          {text}
        </Col>
      </Item>
    )
  }

  renderDetail(detail) {
    if (!detail) return
    const sections = [
      'abstract',
      'motivation',
      'goal',
      'plan',
      'relevance',
      'budget'
    ]

    const metaNode = this.renderMetaNode()
    const titleNode = this.renderTitleNode()
    const labelNode = this.renderLabelNode()
    const tagsNode = this.renderTagsNode()

    let status = I18N.get('suggestion.status.posted')
    if (_.get(detail, 'reference.0.vid')) {
      status = <TagsContainer data={detail} />
    } else if (_.some(detail.tags, (tag) => tag.type === 'INFO_NEEDED')) {
      status = I18N.get('suggestion.status.moreInfoRequired')
    } else if (
      _.some(detail.tags, (tag) => tag.type === 'UNDER_CONSIDERATION')
    ) {
      status = I18N.get('suggestion.status.underConsideration')
    }

    return (
      <div>
        {metaNode}
        {titleNode}
        <div style={{ margin: '14px 0' }}>{labelNode}</div>
        <div>{tagsNode}</div>

        <DescLabel id="preamble">
          {I18N.get('suggestion.fields.preamble')}
        </DescLabel>
        {detail.displayId &&
          this.renderPreambleItem(
            I18N.get('suggestion.fields.preambleSub.suggestion'),
            `#${detail.displayId}`
          )}
        {this.renderPreambleItem(
          I18N.get('suggestion.fields.preambleSub.title'),
          detail.title
        )}
        {detail.createdBy &&
          detail.createdBy.username &&
          this.renderPreambleItem(
            I18N.get('suggestion.fields.preambleSub.creator'),
            detail.createdBy.username,
            'username'
          )}
        {this.renderPreambleItem(
          I18N.get('suggestion.fields.preambleSub.status'),
          status
        )}
        {this.renderPreambleItem(
          I18N.get('suggestion.fields.preambleSub.created'),
          moment(detail.createdAt).format('MMM D, YYYY')
        )}
        {_.get(detail, 'signature.data') &&
          this.renderPreambleItem(
            I18N.get('suggestion.fields.preambleSub.signature'),
            detail.signature.data,
            'signature'
          )}
        {sections.map((section) => {
          if (
            section === 'plan' &&
            detail.plan &&
            typeof detail.plan !== 'string'
          ) {
            return (
              <div key="plan">
                <DescLabel id="plan">
                  {I18N.get('suggestion.fields.plan')}
                </DescLabel>
                <Subtitle>{I18N.get('suggestion.plan.milestones')}</Subtitle>
                {typeof this.state.version !== 'number' ? (
                  <Milestones
                    initialValue={detail.plan.milestone}
                    editable={false}
                  />
                ) : (
                  <MilestonesReadonly
                    initialValue={detail.plan.milestone}
                    editable={false}
                  />
                )}
                <Subtitle>{I18N.get('suggestion.plan.teamInfo')}</Subtitle>
                <TeamInfoList list={detail.plan.teamInfo} editable={false} />
              </div>
            )
          }

          if (
            section === 'budget' &&
            detail.budget &&
            typeof detail.budget !== 'string'
          ) {
            return (
              <div key="budget">
                <DescLabel id="budget">
                  {I18N.get('suggestion.fields.budget')}
                </DescLabel>
                <Subtitle>
                  {`${I18N.get('suggestion.budget.total')} (ELA)`}
                </Subtitle>
                <Paragraph>{detail.budgetAmount}</Paragraph>
                <Subtitle>{I18N.get('suggestion.budget.address')}</Subtitle>
                <Paragraph>{detail.elaAddress}</Paragraph>
                <Subtitle>{I18N.get('suggestion.budget.schedule')}</Subtitle>
                <PaymentList
                  list={detail.budget}
                  milestone={detail.plan.milestone}
                  editable={false}
                />
              </div>
            )
          }

          return (
            <div key={section}>
              <DescLabel id={section}>
                {I18N.get(`suggestion.fields.${section}`)}
              </DescLabel>
              <MarkdownPreview content={detail[section]} />
            </div>
          )
        })}
      </div>
    )
  }

  renderSocialShareButtonsNode() {
    const { detail } = this.props
    return (
      <SocialShareButtons
        shareQuote={`${detail.title} - Suggestion Detail - Cyber Republic`}
      />
    )
  }

  handleShowVersionHistory = () => {
    const id = _.get(this.props, 'match.params.id')
    this.props.history.push(`/suggestion/history/${id}`)
  }

  renderTitleButton = () => {
    const { detail, currentUserId, isAdmin } = this.props
    const isOwner = currentUserId === _.get(detail, 'createdBy._id') || isAdmin
    return (
      isOwner && (
        <Button
          onClick={this.handleShowVersionHistory}
          className="btn-create-suggestion"
          htmlType="button"
          style={{ position: 'relative', top: -5, marginRight: 10 }}
        >
          {I18N.get('suggestion.form.button.showVersion')}
        </Button>
      )
    )
  }

  renderMetaNode() {
    const { detail, user } = this.props
    return (
      <MetaContainer
        data={detail}
        user={user}
        content={this.renderTitleButton()}
      />
    )
  }

  renderTitleNode() {
    const { detail } = this.props
    return <Title>{detail.title}</Title>
  }

  renderLabelNode() {
    const { isReference } = this.props
    if (!isReference) {
      return null
    }
    let result = _.get(this.props.detail, 'reference')
    let reference = _.last(result)
    const { _id, vid, status } = reference
    // when proposal is draft, do not show the label
    if (status === CVOTE_STATUS.DRAFT) return null
    const linkText = `${I18N.get('council.voting.proposal')} #${vid}`
    return (
      <Label style={{ border: 'none' }}>
        {`${I18N.get('suggestion.referred')} `}
        <Link to={`/proposals/${_id}`}>{linkText}</Link>
        {` (${I18N.get(`cvoteStatus.${status}`)})`}
      </Label>
    )
  }

  renderTagsNode() {
    const tags = _.get(this.props.detail, 'tags')
    if (_.isEmpty(tags)) return null
    const res = _.map(tags, (tag) => {
      const { type, _id, desc } = tag
      if (type === SUGGESTION_TAG_TYPE.INFO_NEEDED) {
        return (
          <div key={_id} style={{ display: 'inline' }}>
            <LabelPointer
              type={type}
              data-desc={desc && desc.replace(/(['"])/g, '\\$1')}
              onClick={() => this.setState({ needsInfoVisible: true })}
            >
              {I18N.get(`suggestion.tag.type.${type}`)}
              {'  '}
              <IconWrap>
                <CommentIcon className="more-info-icon" />
              </IconWrap>
            </LabelPointer>
            <Modal
              title={I18N.get(`suggestion.tag.type.${type}`)}
              visible={this.state.needsInfoVisible}
              onCancel={this.closeNeedsInfoModal.bind(this)}
              footer={[
                <Button
                  key="close"
                  onClick={this.closeNeedsInfoModal.bind(this)}
                >
                  Close
                </Button>
              ]}
            >
              <div style={{ fontWeight: 200, paddingBottom: '18px' }}>
                {I18N.get('suggestion.modal.pleaseUpdate')}
              </div>
              {I18N.get('suggestion.modal.commentsFromCouncil')}
              <br />
              <CouncilComments>{desc}</CouncilComments>
            </Modal>
          </div>
        )
      }

      if (type === SUGGESTION_TAG_TYPE.UNDER_CONSIDERATION) {
        return (
          <LabelPointer type={type}>
            {I18N.get(`suggestion.tag.type.${type}`)}
          </LabelPointer>
        )
      }
    })
    return res
  }

  closeNeedsInfoModal() {
    this.setState({
      needsInfoVisible: false
    })
  }

  renderTranslationBtn() {
    const { detail } = this.props
    const sections = [
      'abstract',
      'goal',
      'motivation',
      'plan',
      'relevance',
      'budget'
    ]
    const result = sections
      .map((section) => {
        if (
          section === 'plan' &&
          detail.plan &&
          typeof detail.plan !== 'string'
        ) {
          return `
            <h2>${I18N.get('suggestion.fields.plan')}</h2>
            <p>${getPlanHtml(detail.plan.teamInfo)}</p>
          `
        }
        if (
          section === 'budget' &&
          detail.budget &&
          typeof detail.budget !== 'string'
        ) {
          return `
            <h2>${I18N.get('suggestion.fields.budget')}</h2>
            <p>${I18N.get('suggestion.budget.total')}</p>
            <p>${detail.budgetAmount}</p>
            <p>${I18N.get('suggestion.budget.address')}</p>
            <p>${detail.elaAddress}</p>
            <p>${getBudgetHtml(detail.budget)}</p>
          `
        }
        return `
          <h2>${I18N.get(`suggestion.fields.${section}`)}</h2>
          <p>${convertMarkdownToHtml(
            removeImageFromMarkdown(detail[section])
          )}</p>
          `
      })
      .join('')
    const text = `
      <h3>${detail.title}</h3>
      <br />
      <br />
      ${result}
    `
    return (
      <div style={{ marginTop: 20 }}>
        <Translation text={text} />
      </div>
    )
  }

  renderActionsNode() {
    const { detail } = this.props
    return <ActionsContainer data={detail} />
  }

  onCreated = () => {
    this.refetch()
    this.props.history.push('/proposals')
  }

  renderOwnerActionsNode() {
    const {
      detail,
      currentUserId,
      isAdmin,
      draft,
      getSignatureUrl,
      getSignature
    } = this.props
    const oldData = _.get(detail, 'old')
    const signature = _.get(detail, 'signature.data')
    const isOwner = currentUserId === _.get(detail, 'createdBy._id')
    const isEditable = (isOwner || isAdmin) && !signature
    const editText =
      draft && draft.empty
        ? I18N.get('suggestion.btnText.edit')
        : I18N.get('suggestion.btnText.editDraft')
    const EditButton = isEditable && (
      <div style={{ paddingRight: 16, display: 'inline-block' }}>
        <StyledButton
          type="ebp"
          className="cr-btn cr-btn-default"
          onClick={this.showEditForm}
        >
          {editText}
        </StyledButton>
      </div>
    )
    const isSignable = !signature && isOwner
    return (
      !oldData && (
        <div>
          {EditButton}
          {isSignable && (
            <SignSuggestionButton
              getSignatureUrl={getSignatureUrl}
              getSignature={getSignature}
              id={detail._id}
            />
          )}
        </div>
      )
    )
  }

  renderCouncilActionsNode() {
    const {
      isCouncil,
      isAdmin,
      isReference,
      detail,
      getCMSignatureUrl
    } = this.props
    const oldData = _.get(detail, 'old')
    const signature = _.get(detail, 'signature.data')
    const makeIntoProposalPanel = this.renderMakeIntoProposalPanel()

    const considerBtn = (isCouncil || isAdmin) && (
      <Col xs={24} sm={8}>
        <Popconfirm
          title={I18N.get('suggestion.modal.consideration')}
          onConfirm={() => this.consider()}
          okText={I18N.get('.yes')}
          cancelText={I18N.get('.no')}
        >
          <StyledButton type="ebp" className="cr-btn cr-btn-default">
            {I18N.get('suggestion.btnText.markConsider')}
          </StyledButton>
        </Popconfirm>
      </Col>
    )
    const needMoreInfoBtn = (isCouncil || isAdmin) && (
      <Col xs={24} sm={8}>
        <StyledButton
          type="ebp"
          className="cr-btn cr-btn-default"
          onClick={this.showAddTagModal}
        >
          {I18N.get('suggestion.btnText.needMoreInfo')}
        </StyledButton>
      </Col>
    )
    const makeIntoProposalBtn = signature &&
      isCouncil &&
      !isReference && (
        <Col xs={24} sm={8}>
          <CMSignSuggestionButton
            getCMSignatureUrl={getCMSignatureUrl}
            id={detail._id}
            user={this.props.user}
          />
        </Col>
      )
    const needDueDiligenceBtn = isCouncil && (
      <Col xs={24} sm={8}>
        <StyledButton
          type="ebp"
          className="cr-btn cr-btn-default"
          onClick={this.needDueDiligence}
        >
          {I18N.get('suggestion.btn.needDueDiligence')}
        </StyledButton>
      </Col>
    )
    const needAdvisoryBtn = isCouncil && (
      <Col xs={24} sm={8}>
        <StyledButton
          type="ebp"
          className="cr-btn cr-btn-default"
          onClick={this.needAdvisory}
        >
          {I18N.get('suggestion.btn.needAdvisory')}
        </StyledButton>
      </Col>
    )

    const res = (
      <BtnGroup>
        {makeIntoProposalPanel}
        <Row type="flex" justify="start">
          {considerBtn}
          {needMoreInfoBtn}
          {makeIntoProposalBtn}
        </Row>
        <Row type="flex" justify="start">
          {needDueDiligenceBtn}
          {needAdvisoryBtn}
        </Row>
      </BtnGroup>
    )
    return !oldData && res
  }

  renderMakeIntoProposalPanel() {
    const { isReference, detail } = this.props
    if (!isReference) return null
    let result = _.get(this.props.detail, 'reference')
    let reference = _.last(result)
    const { _id, vid, proposer } = reference
    return (
      <Row style={{ marginBottom: 30 }}>
        <Row type="flex" justify="center" style={{ marginBottom: 15 }}>
          <Col xs={24} sm={8} style={{ textAlign: 'center' }}>
            <StyledButton
              className="cr-btn cr-btn-primary cr-btn-ghost"
              disabled={true}
            >
              {I18N.get('suggestion.btn.makeIntoProposal')}
            </StyledButton>
          </Col>
        </Row>
        <Row type="flex" justify="center">
          <Col span={24}>
            <CreateProposalText>
              {proposer ? proposer : userUtil.formatUsername(detail.proposer)}{' '}
              {I18N.get('suggestion.label.hasMadeIntoProposal')}
              <Link to={`/proposals/${_id}`}>
                {` ${I18N.get('council.voting.proposal')} #${vid}`}
              </Link>
            </CreateProposalText>
          </Col>
        </Row>
      </Row>
    )
  }

  renderCommentNode() {
    const { detail } = this.props
    const oldData = _.get(detail, 'old')
    return (
      <Comments
        id="comments"
        type="suggestion"
        suggestion={detail}
        canPost={!oldData}
        model={detail._id}
        returnUrl={`/suggestion/${detail._id}`}
      />
    )
  }

  consider = async () => {
    const { _id } = this.props.detail
    try {
      await this.props.addTag({
        id: _id,
        type: SUGGESTION_TAG_TYPE.UNDER_CONSIDERATION
      })
      this.refetch()
      message.success(I18N.get('suggestion.msg.consideration'))
    } catch (error) {
      logger.error(error)
    }
  }

  needMoreInfo = async () => {
    const { comment } = this.state
    const { _id } = this.props.detail
    try {
      await this.props.addTag({
        id: _id,
        type: SUGGESTION_TAG_TYPE.INFO_NEEDED,
        desc: comment
      })
      this.refetch()
    } catch (error) {
      logger.error(error)
    }
  }

  showAddTagModal = () => {
    Modal.confirm({
      title: I18N.get('suggestion.modal.addTagComment'),
      content: <TextArea onChange={this.onCommentChanged} />,
      okText: I18N.get('suggestion.modal.confirm'),
      cancelText: I18N.get('suggestion.modal.cancel'),
      onOk: () => this.needMoreInfo()
    })
  }

  onCommentChanged = (e) => {
    this.setState({ comment: e.target.value })
  }

  onFormSubmit = async (param) => {
    try {
      await this.props.update(param)
      this.showEditForm()
      this.refetch()
    } catch (error) {
      logger.error(error)
    }
  }

  renderEditForm = () => {
    const { detail } = this.props

    const props = {
      onFormCancel: this.showEditForm,
      onFormSubmit: this.onFormSubmit,
      header: I18N.get('suggestion.header.edit'),
      data: detail
    }

    return (
      <Modal
        className="project-detail-nobar"
        maskClosable={false}
        visible={this.state.showForm}
        onOk={this.showEditForm}
        onCancel={this.showEditForm}
        footer={null}
        width="70%"
      >
        {this.state.showForm && <SuggestionForm {...props} />}
      </Modal>
    )
  }

  showEditForm = () => {
    const id = _.get(this.props, 'match.params.id')
    this.props.history.push(`/suggestion/${id}/edit`)
  }

  showDropdownActions = () => {
    const { isDropdownActionOpen } = this.state
    this.setState({
      isDropdownActionOpen: !isDropdownActionOpen
    })
  }

  refetch = async (incViewsNum) => {
    const id = _.get(this.props, 'match.params.id')
    await this.props.resetDetail()
    this.props.getDetail({ id, incViewsNum: !!incViewsNum })
  }

  linkSuggestionDetail(suggestionId) {
    this.props.history.push(`/suggestion/${suggestionId}`)
  }

  needDueDiligence = async () => {
    const { _id } = this.props.detail
    await this.props.needDueDiligence(_id)
    message.success(I18N.get('suggestion.msg.notify'))
  }

  needAdvisory = async () => {
    const { _id } = this.props.detail
    await this.props.needAdvisory(_id)
    message.success(I18N.get('suggestion.msg.notify'))
  }
}
