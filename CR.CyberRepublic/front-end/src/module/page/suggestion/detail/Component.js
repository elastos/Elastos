import React from 'react'
import ReactDOMServer from 'react-dom/server'
import _ from 'lodash'
import { Row, Col, Spin, Divider, Modal, Input, Button, Icon } from 'antd'
import { Link } from 'react-router-dom'
import MediaQuery from 'react-responsive'
import moment from 'moment/moment'
import Comments from '@/module/common/comments/Container'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import Translation from '@/module/common/Translation/Container'
import SuggestionForm from '@/module/form/SuggestionForm/Container'
import ProposalForm from '@/module/page/CVote/create/Container'
import I18N from '@/I18N'
import { LG_WIDTH } from '@/config/constant'
import { CVOTE_STATUS, SUGGESTION_TAG_TYPE } from '@/constant'
import sanitizeHtml from 'sanitize-html'
import StandardPage from '../../StandardPage'
import ActionsContainer from '../common/actions/Container'
import MetaContainer from '../common/meta/Container'
import MySuggestion from '../my_list/Container'

import { ReactComponent as CommentIcon } from '@/assets/images/icon-info.svg'

import { Container, Title, ShortDesc, DescLabel, Label, LabelPointer, Desc, BtnGroup, StyledButton, DescBody, CouncilComments, IconWrap } from './style'

import './style.scss'

const { TextArea } = Input

export default class extends StandardPage {
  constructor(props) {
    super(props)

    // we use the props from the redux store if its retained
    this.state = {
      isDropdownActionOpen: false,
      showMobile: false,
      showForm: false,
      needsInfoVisible: false
    }
  }

  componentDidMount() {
    super.componentDidMount()
    this.refetch(true)
  }

  componentWillUnmount() {
    this.props.resetDetail()
  }

  ord_renderContent() {
    if (_.isEmpty(this.props.detail) || this.props.detail.loading) {
      return <div className="center"><Spin size="large" /></div>
    }
    const detailNode = this.renderDetail()
    const translationBtn = this.renderTranslationBtn()
    const actionsNode = this.renderActionsNode()
    const ownerActionsNode = this.renderOwnerActionsNode()
    const councilActionsNode = this.renderCouncilActionsNode()
    const editForm = this.renderEditForm()
    // const mySuggestionNode = <MySuggestion />
    const commentNode = this.renderCommentNode()
    return (
      <div>
        <Container className="c_SuggestionDetail">
          <MediaQuery maxWidth={LG_WIDTH}>
            <div><BackLink link="/suggestion" style={{ position: 'relative', left: 0, marginBottom: 15 }} /></div>
            <div>
              {detailNode}
              {translationBtn}
              {actionsNode}
              {ownerActionsNode}
              {councilActionsNode}
            </div>
            {/* <div>{mySuggestionNode}</div> */}
            <div style={{ marginTop: 60 }}>{commentNode}</div>
          </MediaQuery>
          <MediaQuery minWidth={LG_WIDTH + 1}>
            <BackLink link="/suggestion" />
            <Row gutter={24}>
              <Col span={24}>
                {detailNode}
                {translationBtn}
                {actionsNode}
                {ownerActionsNode}
                {councilActionsNode}
                <div style={{ marginTop: 60 }}>{commentNode}</div>
              </Col>
              {/* <Col span={9}>{mySuggestionNode}</Col> */}
            </Row>
          </MediaQuery>
          {editForm}
        </Container>
        <Footer />
      </div>
    )
  }

  renderDetail() {
    const metaNode = this.renderMetaNode()
    const titleNode = this.renderTitleNode()
    const shortDescNode = this.renderShortDescNode()
    const labelNode = this.renderLabelNode()
    const tagsNode = this.renderTagsNode()
    const descNode = this.renderDescNode()
    const benefitsNode = this.renderBenefitsNode()
    const fundingNode = this.renderFundingNode()
    const timelineNode = this.renderTimelineNode()
    const linkNode = this.renderLinkNode()
    return (
      <div>
        {metaNode}
        {titleNode}
        {/* labelNode */}
        {tagsNode}
        {shortDescNode}
        <Divider/>
        {descNode}
        <Divider/>
        {benefitsNode}
        {fundingNode}
        {timelineNode}
        {linkNode}
      </div>
    )
  }

  renderMetaNode() {
    const { detail } = this.props
    return <MetaContainer data={detail} />
  }

  renderTitleNode() {
    const { detail } = this.props
    return (
      <Title>{detail.title}</Title>
    )
  }

  renderShortDescNode() {
    const { detail } = this.props
    return (
      <ShortDesc>
        {detail.shortDesc}
      </ShortDesc>
    )
  }

  renderLabelNode() {
    const reference = _.get(this.props.detail, 'reference')
    if (_.isEmpty(reference)) return null
    const { _id, vid, status } = _.last(reference)
    // when proposal is draft, do not show the label
    if (status === CVOTE_STATUS.DRAFT) return null
    const linkText = `${I18N.get('council.voting.proposal')} #${vid}`
    return (
      <Label>
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
      return (
        <div key={_id} >
          {
            type === SUGGESTION_TAG_TYPE.INFO_NEEDED &&
              <LabelPointer type={type} data-desc={desc.replace(/(['"])/g, '\\$1')}
                            onClick={() => this.setState({needsInfoVisible: true})}>
                {I18N.get(`suggestion.tag.type.${type}`)} &nbsp;
                <IconWrap>
                  <CommentIcon className="more-info-icon"/>
                </IconWrap>
              </LabelPointer>
          }
          <Modal
            title={I18N.get(`suggestion.tag.type.${type}`)}
            visible={this.state.needsInfoVisible}
            onCancel={this.closeNeedsInfoModal.bind(this)}
            footer={[
              <Button key="close" onClick={this.closeNeedsInfoModal.bind(this)}>Close</Button>
            ]}
          >
            <div style={{fontWeight: 200, paddingBottom: '18px'}}>
              {I18N.get('suggestion.modal.pleaseUpdate')}
            </div>
            {I18N.get('suggestion.modal.commentsFromCouncil')}<br/>
            <CouncilComments>
              {desc}
            </CouncilComments>
          </Modal>
        </div>
      )
    })
    return res
  }

  closeNeedsInfoModal() {
    this.setState({
      needsInfoVisible: false

    })
  }

  renderDescNode() {
    const { detail } = this.props
    return (
      <Desc>
        <DescLabel>{I18N.get('suggestion.form.fields.fullDesc')}</DescLabel>
        <DescBody className="ql-editor" dangerouslySetInnerHTML={{ __html: sanitizeHtml(detail.desc) }} />
      </Desc>
    )
  }

  renderBenefitsNode() {
    const { detail } = this.props
    if (!detail.benefits) {
      return null
    }
    return (
      <Desc>
        <DescLabel>{I18N.get('suggestion.form.fields.benefits')}</DescLabel>
        <DescBody>{detail.benefits}</DescBody>
      </Desc>
    )
  }

  renderFundingNode() {
    const { detail } = this.props
    if (!detail.funding) {
      return null
    }
    return (
      <Desc>
        <DescLabel>{I18N.get('suggestion.form.fields.funding')}</DescLabel>
        <div>{detail.funding}</div>
      </Desc>
    )
  }

  renderTimelineNode() {
    const { detail } = this.props
    if (!detail.timeline) {
      return null
    }
    return (
      <Desc>
        <DescLabel>{I18N.get('suggestion.form.fields.timeline')}</DescLabel>
        <div>{moment(detail.timeline).format('MMM D, YYYY')}</div>
      </Desc>
    )
  }

  renderLinkNode() {
    const { link } = this.props.detail

    if (_.isEmpty(link || _.isEmpty(_.get(link, '[0]')))) {
      return null
    }

    return (
      <Desc>
        <DescLabel>{I18N.get('suggestion.form.fields.links')}</DescLabel>
        {_.map(link, href => <div key={href}><a href={href} target="_blank" rel="noopener noreferrer">{href}</a></div>)}
      </Desc>
    )
  }

  renderTranslationBtn() {
    const { title, desc, benefits } = this.props.detail
    const text = `
      <h1>${title}</h1>
      <h4>${I18N.get('suggestion.form.fields.desc')}</h4>
      ${desc}
      ${benefits ? `<h4>${I18N.get('suggestion.form.fields.benefits')}</h4>
      <p>${benefits}</p>` : ''}
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
    const { detail, currentUserId, isAdmin } = this.props
    const isOwner = currentUserId === _.get(detail, 'createdBy._id') || isAdmin
    const res = isOwner && (
      <StyledButton type="ebp" className="cr-btn cr-btn-default" onClick={this.showEditForm}>
        {I18N.get('suggestion.btnText.edit')}
      </StyledButton>
    )
    return res
  }

  renderCouncilActionsNode() {
    const { isCouncil, isAdmin, detail } = this.props
    const { _id, displayId, title } = detail
    const descNode = this.renderDescNode()
    const benefitsNode = this.renderBenefitsNode()
    const fundingNode = this.renderFundingNode()
    const timelineNode = this.renderTimelineNode()
    const linkNode = this.renderLinkNode()

    const proposalContent = `
      ${ReactDOMServer.renderToString(descNode)}
      ${ReactDOMServer.renderToString(benefitsNode)}
      ${ReactDOMServer.renderToString(fundingNode)}
      ${ReactDOMServer.renderToString(timelineNode)}
      ${ReactDOMServer.renderToString(linkNode)}
    `

    const props = {
      data: {
        title,
        content: proposalContent,
      },
      suggestionDisplayId: displayId,
      suggestionId: _id,
      onCreated: this.onCreated,
      btnText: I18N.get('suggestion.btnText.makeIntoProposal'),
      btnStyle: { width: 200 },
    }
    const considerBtn = (isCouncil || isAdmin) && (
      <Col xs={24} sm={8}>
        <StyledButton type="ebp" className="cr-btn cr-btn-default" onClick={this.consider}>
          {I18N.get('suggestion.btnText.markConsider')}
        </StyledButton>
      </Col>
    )
    const needMoreInfoBtn = (isCouncil || isAdmin) && (
      <Col xs={24} sm={8}>
        <StyledButton type="ebp" className="cr-btn cr-btn-default" onClick={this.showAddTagModal}>
          {I18N.get('suggestion.btnText.needMoreInfo')}
        </StyledButton>
      </Col>
    )
    const createFormBtn = isCouncil && (
      <Col xs={24} sm={8}>
        <ProposalForm {...props} />
      </Col>
    )

    const res = (
      <BtnGroup>
        <Row type="flex" justify="start">
          {considerBtn}
          {needMoreInfoBtn}
          {createFormBtn}
        </Row>
      </BtnGroup>
    )
    return res
  }

  renderCommentNode() {
    const { detail } = this.props
    return (
      <Comments
        type="suggestion"
        suggestion={detail}
        canPost={true}
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
        type: SUGGESTION_TAG_TYPE.UNDER_CONSIDERATION,
      })
      this.refetch()
    } catch (error) {
      // console.log(error)
    }
  }

  needMoreInfo = async () => {
    const { comment } = this.state
    const { _id } = this.props.detail
    try {
      await this.props.addTag({
        id: _id,
        type: SUGGESTION_TAG_TYPE.INFO_NEEDED,
        desc: comment,
      })
      // this.showAddTagModal()
      this.refetch()
    } catch (error) {
      // console.log(error)
    }
  }

  showAddTagModal = () => {
    Modal.confirm({
      title: I18N.get('suggestion.modal.addTagComment'),
      content: <TextArea onChange={this.onCommentChanged} />,
      okText: I18N.get('suggestion.modal.confirm'),
      cancelText: I18N.get('suggestion.modal.cancel'),
      onOk: () => this.needMoreInfo(),
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
      // console.log(error)
    }
  }

  renderEditForm = () => {
    const { detail } = this.props

    const props = {
      onFormCancel: this.showEditForm,
      onFormSubmit: this.onFormSubmit,
      header: I18N.get('suggestion.header.edit'),
      data: detail,
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
        { this.state.showForm
          && <SuggestionForm {...props} />
        }
      </Modal>
    )
  }

  showEditForm = () => {
    const { showForm } = this.state

    this.setState({
      showForm: !showForm,
    })
  }

  showDropdownActions = () => {
    const { isDropdownActionOpen } = this.state
    this.setState({
      isDropdownActionOpen: !isDropdownActionOpen,
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
}
