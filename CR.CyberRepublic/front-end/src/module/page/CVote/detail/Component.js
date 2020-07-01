import React from 'react'
import {
  Spin,
  Button,
  Input,
  message,
  Modal,
  Anchor,
  Row,
  Col,
  Popconfirm
} from 'antd'
import { Link } from 'react-router-dom'
import I18N from '@/I18N'
import _ from 'lodash'
import StandardPage from '@/module/page/StandardPage'
import { CVOTE_RESULT, CVOTE_STATUS, ELIP_TYPE } from '@/constant'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import CRPopover from '@/module/shared/Popover/Component'
import MetaComponent from '@/module/shared/meta/Container'
import Translation from '@/module/common/Translation/Container'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import { StickyContainer, Sticky } from 'react-sticky'
import VoteResultComponent from '../common/vote_result/Component'
import MemberVoteQrCode from './MemberVoteQrCode'
import Preamble from './Preamble'
import ElipPreamble from './ElipPreamble'
import Tracking from '../tracking/Container'
import Summary from '../summary/Container'
import Meta from '@/module/common/Meta'
import SocialShareButtons from '@/module/common/SocialShareButtons'
import { logger } from '@/util'

import {
  convertMarkdownToHtml,
  removeImageFromMarkdown,
  getPlanHtml,
  getBudgetHtml
} from '@/util/markdown-it'
import PaymentList from './PaymentList'
import TeamInfoList from '@/module/form/SuggestionForm/TeamInfoList'
import Milestones from '@/module/form/SuggestionForm/Milestones'
import {
  Container,
  Title,
  Label,
  ContentTitle,
  StyledAnchor,
  FixedHeader,
  FixedHeaderQrCode,
  Body,
  SubTitleHeading,
  SubTitleContainer,
  VoteBtnGroup,
  StyledRichContent,
  LinkGroup,
  Part,
  PartTitle,
  PartContent,
  Subtitle,
  Paragraph
} from './style'
import './style.scss'

const { TextArea } = Input

const renderRichContent = (data, key, title, user, actions) => {
  let rc
  if (key === 'budget' && data.budget && typeof data.budget !== 'string') {
    rc = (
      <div>
        <Subtitle>{`${I18N.get('suggestion.budget.total')} (ELA)`}</Subtitle>
        <Paragraph>{data.budgetAmount}</Paragraph>
        <Subtitle>{I18N.get('suggestion.budget.address')}</Subtitle>
        <Paragraph>{data.elaAddress}</Paragraph>
        <Subtitle>{I18N.get('suggestion.budget.schedule')}</Subtitle>
        <div className='budget-payment-list'>
          <PaymentList
            list={data.budget}
            milestone={data.plan.milestone}
            withdrawalHistory={data.withdrawalHistory}
            user={user}
            proposer={data.proposer}
            proposalId={data._id}
            actions={actions}
            status={data.status}
          />
        </div>

      </div>
    )
  } else if (key === 'plan' && data.plan && typeof data.plan !== 'string') {
    rc = (
      <div>
        <Subtitle>{I18N.get('suggestion.plan.milestones')}</Subtitle>
        <Milestones initialValue={data.plan.milestone} editable={false} />
        <Subtitle>{I18N.get('suggestion.plan.teamInfo')}</Subtitle>
        <TeamInfoList list={data.plan.teamInfo} editable={false} />
      </div>
    )
  } else {
    rc = <MarkdownPreview content={data[key]} />
  }
  return (
    <div>
      {title && <ContentTitle id={key}>{title}</ContentTitle>}
      <StyledRichContent>{rc}</StyledRichContent>
    </div>
  )
}

class C extends StandardPage {
  constructor(props) {
    super(props)

    this.state = {
      persist: true,
      loading: false,
      visibleYes: false,
      visibleOppose: false,
      visibleAbstain: false,
      editing: false,
      smallSpace: false
    }

    this.isLogin = this.props.isLogin
    this.user = this.props.user
  }

  async componentDidMount() {
    await this.refetch()
  }

  componentWillUnmount() {
    this.props.resetData()
  }

  refetch = async () => {
    const param = {
      id: _.get(this.props.match, 'params.id')
    }
    await this.props.getData(param)
  }

  ord_loading(f = false) {
    this.setState({ loading: f })
  }

  ord_renderMeta(f = false) {
    return f
  }

  ord_renderContent() {
    const { data, currentUserId } = this.props
    if (!data) {
      return (
        <div className="center">
          <Spin />
        </div>
      )
    }
    if (data && data.success && data.empty) {
      return (
        <div className="ebp-page">
          <h1>{I18N.get('error.notfound')}</h1>
        </div>
      )
    }

    const currentVoted = _.find(data.voteResult, function (o) {
      if (o.votedBy != null) {
        if (o.votedBy._id == currentUserId) {
          return o
        }
      }
    })
    const isVote = currentVoted && (currentVoted.status == 'chained' || currentVoted.value == 'undecided')
    const anchorNode = this.renderAnchor()
    const contentNode = this.renderContent()
    const translationBtn = this.renderTranslationBtn()
    const notesNode = this.renderNotes()
    const voteActionsNode = this.renderVoteActions()
    const voteDetailNode = this.renderVoteResults()
    const trackingNode = this.renderTracking()
    const summaryNode = this.renderSummary()

    // get the first line pure text of abstract
    const abstract = data.abstract && data.abstract.trim().split('\n')[0]

    return (
      <div>
        <Meta
          desc={abstract}
          title={`${data.title} - Proposal Detail - Cyber Republic`}
          url={this.props.location.pathname}
        />
        {anchorNode}
        <Container className="p_CVoteDetail">
          <StickyContainer>
            <BackLink link="/proposals" />
            {this.renderStickyHeader()}
            <Body>
              {contentNode}
              {translationBtn}
              {notesNode}
              {isVote ? voteActionsNode : null}
              {voteDetailNode}
              {trackingNode}
              {summaryNode}
            </Body>
            <SocialShareButtons
              shareQuote={`${data.title} - Proposal Detail - Cyber Republic`}
            />
          </StickyContainer>
        </Container>
        <Footer />
      </div>
    )
  }

  renderStickyHeader() {
    const metaNode = this.renderMeta()
    const titleNode = this.renderTitle()
    const labelNode = this.renderLabelNode()
    const subTitleNode = this.renderSubTitle()
    const memberVoteNode = this.renderMemberVoteQrCode()
    const { smallSpace } = this.state
    return (
      <Sticky>
        {({ style, isSticky, wasSticky }) => {
          if (!wasSticky && isSticky && !smallSpace) {
            this.setState({ smallSpace: true })
          }
          if (wasSticky && !isSticky && smallSpace) {
            this.setState({ smallSpace: false })
          }
          const finalStyle = style
            ? {
              ...style,
              zIndex: 2
            }
            : style
          const isNotification = this.props.data.status == 'NOTIFICATION'
          return (
            <div style={finalStyle}>
              <Row>
                <Col span={isNotification ? 12 : 24}>
                  <FixedHeader>
                    {metaNode}
                    {titleNode}
                    {labelNode}
                    {subTitleNode}
                  </FixedHeader>
                </Col>
                {isNotification ? (
                  <Col span={12}>
                    <FixedHeader>{memberVoteNode}</FixedHeader>
                  </Col>
                ) : null}
              </Row>
            </div>
          )
        }}
      </Sticky>
    )
  }

  renderMemberVoteQrCode() {
    const { data, getMemberVoteUrl } = this.props
    return <MemberVoteQrCode {...data} getMemberVoteUrl={getMemberVoteUrl} />
  }

  renderTranslationBtn() {
    const { data, isElip } = this.props
    const { title, content } = data
    let text = ''
    if (content) {
      text = `<h3>${title}</h3><br /><br /> ${content}`
      return (
        <div style={{ marginTop: 20 }}>
          <Translation text={text} />
        </div>
      )
    }
    let sections, result
    if (isElip) {
      sections = [
        'abstract',
        'motivation',
        'specification',
        'rationale',
        'backwardCompatibility',
        'referenceImplementation',
        'copyright'
      ]
      result = sections
        .map((section) => {
          return `
          <h2>${I18N.get(`elip.fields.${section}`)}</h2>
          <p>${convertMarkdownToHtml(
            removeImageFromMarkdown(data[section])
          )}</p>
        `
        })
        .join('')
    } else {
      sections = [
        'abstract',
        'motivation',
        'goal',
        'plan',
        'relevance',
        'budget'
      ]
      result = sections
        .map((section) => {
          if (
            section === 'budget' &&
            data.budget &&
            typeof data.budget !== 'string'
          ) {
            return `
            <h2>${I18N.get('proposal.fields.budget')}</h2>
            <p>${I18N.get('suggestion.budget.total')}</p>
            <p>${data.budgetAmount}</p>
            <p>${I18N.get('suggestion.budget.address')}</p>
            <p>${data.elaAddress}</p>
            <p>${getBudgetHtml(data.budget)}</p>
          `
          }
          if (
            section === 'plan' &&
            data.plan &&
            typeof data.plan !== 'string'
          ) {
            return `
            <h2>${I18N.get('proposal.fields.plan')}</h2>
            <p>${getPlanHtml(data.plan.teamInfo)}</p>
          `
          }
          return `
          <h2>${I18N.get(`proposal.fields.${section}`)}</h2>
          <p>${convertMarkdownToHtml(
            removeImageFromMarkdown(data[section])
          )}</p>
        `
        })
        .join('')
    }

    text = `<h3>${title}</h3><br /><br/> ${result}`
    return (
      <div style={{ marginTop: 20 }}>
        <Translation text={text} />
      </div>
    )
  }

  renderMeta() {
    const { data, user } = this.props
    data.displayId = data.vid
    const postedByText = I18N.get('from.CVoteForm.label.proposedby')
    return <MetaComponent data={data} postedByText={postedByText} user={user} />
  }

  renderAnchor() {
    const { data, currentUserId, isElip } = this.props
    const { trackingStatus, summaryStatus } = this.props
    let isShowFollowingUp = _.includes(
      [CVOTE_STATUS.ACTIVE, CVOTE_STATUS.INCOMPLETED, CVOTE_STATUS.FINAL],
      data.status
    )
    if (
      data.status === CVOTE_STATUS.PROPOSED &&
      currentUserId === _.get(data, 'proposer._id')
    ) {
      isShowFollowingUp = true
    }

    const trackingTitle = trackingStatus ? (
      <span>
        {I18N.get('proposal.fields.tracking')}{' '}
        <span style={{ fontSize: 10, color: '#aaa' }}>
          ({I18N.get(`proposal.status.trackingRaw.${trackingStatus}`)})
        </span>
      </span>
    ) : (
        I18N.get('proposal.fields.tracking')
      )
    const summaryTitle = summaryStatus ? (
      <span>
        {I18N.get('proposal.fields.summary')}{' '}
        <span style={{ fontSize: 10, color: '#aaa' }}>
          ({I18N.get(`proposal.status.summaryRaw.${summaryStatus}`)})
        </span>
      </span>
    ) : (
        I18N.get('proposal.fields.summary')
      )
    const tracking = isShowFollowingUp && (
      <Anchor.Link href="#tracking" title={trackingTitle} key="tracking" />
    )
    const summary = isShowFollowingUp && (
      <Anchor.Link href="#summary" title={summaryTitle} key="summary" />
    )
    const commonLinks = [tracking, summary]
    return isElip
      ? this.renderElipLinks(commonLinks)
      : this.renderSuggestionLinks(commonLinks)
  }

  renderElipLinks(commonLinks) {
    return (
      <StyledAnchor offsetTop={300}>
        <Anchor.Link
          href="#preamble"
          title={I18N.get('elip.fields.preamble')}
        />
        <Anchor.Link
          href="#abstract"
          title={I18N.get('elip.fields.abstract')}
        />
        <LinkGroup marginTop={48}>
          <Anchor.Link
            href="#motivation"
            title={I18N.get('elip.fields.motivation')}
          />
          <Anchor.Link
            href="#specification"
            title={I18N.get('elip.fields.specification')}
          />
          <Anchor.Link
            href="#rationale"
            title={I18N.get('elip.fields.rationale')}
          />
        </LinkGroup>
        <LinkGroup marginTop={48}>
          <Anchor.Link
            href="#backwardCompatibility"
            title={I18N.get('elip.fields.backwardCompatibility')}
          />
          <Anchor.Link
            href="#referenceImplementation"
            title={I18N.get('elip.fields.referenceImplementation')}
          />
          <Anchor.Link
            href="#copyright"
            title={I18N.get('elip.fields.copyright')}
          />
        </LinkGroup>
        <LinkGroup marginTop={48}>
          <Anchor.Link href="#vote" title={I18N.get('proposal.fields.vote')} />
        </LinkGroup>
        {_.each(commonLinks, (e) => e)}
      </StyledAnchor>
    )
  }

  renderSuggestionLinks(commonLinks) {
    return (
      <StyledAnchor offsetTop={300}>
        <Anchor.Link
          href="#preamble"
          title={I18N.get('proposal.fields.preamble')}
        />
        <Anchor.Link
          href="#abstract"
          title={I18N.get('proposal.fields.abstract')}
        />
        <Anchor.Link
          marginTop={48}
          href="#motivation"
          title={I18N.get('proposal.fields.motivation')}
        />
        <LinkGroup>
          <Anchor.Link href="#goal" title={I18N.get('proposal.fields.goal')} />
        </LinkGroup>
        <Anchor.Link href="#plan" title={I18N.get('proposal.fields.plan')} />
        <Anchor.Link
          href="#relevance"
          title={I18N.get('proposal.fields.relevance')}
        />
        <LinkGroup marginTop={48}>
          <Anchor.Link
            href="#budget"
            title={I18N.get('proposal.fields.budget')}
          />
        </LinkGroup>
        <LinkGroup marginTop={48}>
          <Anchor.Link href="#vote" title={I18N.get('proposal.fields.vote')} />
        </LinkGroup>
        {_.each(commonLinks, (e) => e)}
      </StyledAnchor>
    )
  }

  renderTitle() {
    const { title } = this.props.data
    return <Title smallSpace={this.state.smallSpace}>{title}</Title>
  }

  renderSubTitle() {
    const status = this.renderStatus()
    // const btns = this.renderAdminActions()
    return (
      <SubTitleContainer>
        {status}
        {/* {btns} */}
      </SubTitleContainer>
    )
  }

  renderStatus() {
    const { data } = this.props
    const statusObj = {
      text: I18N.get('from.CVoteForm.label.voteStatus'),
      value: I18N.get(`cvoteStatus.${data.status}`) || ''
    }

    const result = (
      <SubTitleHeading smallSpace={this.state.smallSpace}>
        <div className="text">{statusObj.text}</div>
        <div className="value">{statusObj.value}</div>
      </SubTitleHeading>
    )
    return result
  }

  renderLabelNode() {
    const { isElip } = this.props
    const reference = isElip
      ? _.get(this.props.data, 'referenceElip')
      : _.get(this.props.data, 'reference')
    if (_.isEmpty(reference)) return null
    let linkText
    let linkUrl
    const { _id, displayId, vid } = reference
    if (isElip) {
      linkText = `${I18N.get('elip.elip')} #${vid}`
      linkUrl = isElip ? `/elips/${_id}` : `/suggestion/${_id}`
    } else {
      linkText = `${I18N.get('suggestion.suggestion')} #${displayId}`
      linkUrl = isElip ? `/elips/${_id}` : `/suggestion/${_id}`
    }
    return (
      <Label>
        {`${I18N.get('council.voting.referred')} `}
        <Link to={linkUrl}>{linkText}</Link>
      </Label>
    )
  }

  renderContent() {
    const { data, user, isElip } = this.props
    if (isElip) {
      const sections = [
        {
          id: 'abstract',
          valueKey: 'abstract'
        },
        {
          id: 'motivation',
          valueKey: 'motivation'
        },
        {
          id: 'specification',
          valueKey: 'specifications'
        },
        {
          id: 'rationale',
          valueKey: 'rationale'
        },
        {
          id: 'backwardCompatibility',
          valueKey: 'backwardCompatibility'
        },
        {
          id: 'referenceImplementation',
          valueKey: 'referenceImplementation'
        },
        {
          id: 'copyright',
          valueKey: 'copyright'
        }
      ]
      return (
        <div>
          <ElipPreamble {...data} user={user} />
          {_.map(sections, (section) => (
            <Part id={section.id} key={section.id}>
              <PartTitle>{I18N.get(`elip.fields.${section.id}`)}</PartTitle>
              <PartContent>
                <MarkdownPreview
                  content={data[section.valueKey] ? data[section.valueKey] : ''}
                />
              </PartContent>
            </Part>
          ))}
        </div>
      )
    }
    // legacy data structure has content field
    if (_.has(data, 'content')) return renderRichContent(data, 'content')
    return (
      <div>
        <Preamble {...data} user={user} />
        {renderRichContent(
          data,
          'abstract',
          I18N.get('proposal.fields.abstract')
        )}
        {renderRichContent(
          data,
          'motivation',
          I18N.get('proposal.fields.motivation')
        )}
        {renderRichContent(data, 'goal', I18N.get('proposal.fields.goal'))}
        {renderRichContent(data, 'plan', I18N.get('proposal.fields.plan'))}
        {renderRichContent(
          data,
          'relevance',
          I18N.get('proposal.fields.relevance')
        )}
        {renderRichContent(
          data,
          'budget',
          I18N.get('proposal.fields.budget'),
          user,
          {
            applyPayment: this.props.applyPayment,
            getPaymentSignature: this.props.getPaymentSignature,
            reviewApplication: this.props.reviewApplication,
            withdraw: this.props.withdraw
          }
        )}
      </div>
    )
  }

  renderNotes() {
    const { notes, notes_zh } = this.props.data
    if (!notes && !notes_zh) return null
    return (
      <div className="content notes">
        <h4 className="notes-title">
          {I18N.get('council.voting.btnText.notesSecretary')}
        </h4>
        <p className="notes-content">{notes}</p>
        <p className="notes-content">{notes_zh}</p>
      </div>
    )
  }

  renderVoteActions() {
    const { isCouncil } = this.props
    const { status } = this.props.data
    const canVote = isCouncil && status === CVOTE_STATUS.PROPOSED

    if (!canVote) return null
    const { visibleYes, visibleOppose, visibleAbstain } = this.state
    const btnYes = (
      <Button
        type="primary"
        icon="check-circle"
        onClick={this.showVoteYesModal}
      >
        {I18N.get('council.voting.btnText.yes')}
      </Button>
    )
    const btnOppose = (
      <Button
        type="danger"
        icon="close-circle"
        onClick={this.showVoteOpposeModal}
      >
        {I18N.get('council.voting.btnText.no')}
      </Button>
    )
    const btnAbstain = (
      <Button icon="stop" onClick={this.showVoteAbstainModal}>
        {I18N.get('council.voting.btnText.abstention')}
      </Button>
    )

    const popOverYes = (
      <CRPopover
        triggeredBy={btnYes}
        visible={visibleYes}
        onToggle={this.showVoteYesModal}
        onSubmit={this.voteYes}
        btnType="primary"
        required={true}
        requiredMsg={I18N.get('from.CVoteForm.reason.yes.required')}
      />
    )
    const popOverOppose = (
      <CRPopover
        triggeredBy={btnOppose}
        visible={visibleOppose}
        onToggle={this.showVoteOpposeModal}
        onSubmit={this.voteOppose}
        btnType="danger"
        required={true}
        requiredMsg={I18N.get('from.CVoteForm.reason.oppose.required')}
      />
    )
    const popOverAbstain = (
      <CRPopover
        triggeredBy={btnAbstain}
        visible={visibleAbstain}
        onToggle={this.showVoteAbstainModal}
        onSubmit={this.voteAbstain}
        required={true}
        requiredMsg={I18N.get('from.CVoteForm.reason.abstain.required')}
      />
    )
    const oldData = _.get(this.props.data, 'old')
    return !oldData && (
      <VoteBtnGroup>
        {popOverYes}
        {popOverOppose}
        {popOverAbstain}
      </VoteBtnGroup>
    )
  }

  renderAdminActions() {
    const { isSecretary, isCouncil, currentUserId } = this.props
    const { status, createdBy } = this.props.data
    const isSelf = currentUserId === createdBy._id
    const isCompleted = _.includes(
      [CVOTE_STATUS.FINAL, CVOTE_STATUS.INCOMPLETED],
      status
    )
    const canManage = isSecretary || isCouncil
    const canEdit = _.includes([CVOTE_STATUS.DRAFT], status)

    if (!canManage || isCompleted) return null

    const editProposalBtn = isSelf &&
      canEdit && (
        <Button onClick={this.gotoEditPage}>
          {I18N.get('council.voting.btnText.editProposal')}
        </Button>
      )
    const publishProposalBtn = isSelf &&
      canEdit && (
        <Button type="primary" onClick={this.publish}>
          {I18N.get('council.voting.btnText.publish')}
        </Button>
      )
    const deleteDraftProposalBtn = isSelf &&
      canEdit && (
        <Popconfirm
          title={I18N.get('council.voting.modal.deleteDraft')}
          onConfirm={() => this.deleteDraftProposal()}
          okText={I18N.get('.yes')}
          cancelText={I18N.get('.no')}
        >
          <Button type="danger">
            {I18N.get('council.voting.btnText.delete')}
          </Button>
        </Popconfirm>
      )
    return (
      <div className="vote-btn-group">
        {editProposalBtn}
        {publishProposalBtn}
        {deleteDraftProposalBtn}
      </div>
    )
  }

  renderTracking() {
    const { data, currentUserId } = this.props
    let isShowFollowingUp = _.includes(
      [CVOTE_STATUS.ACTIVE, CVOTE_STATUS.INCOMPLETED, CVOTE_STATUS.FINAL],
      data.status
    )
    if (
      data.status === CVOTE_STATUS.PROPOSED &&
      currentUserId === _.get(data, 'proposer._id')
    ) {
      isShowFollowingUp = true
    }
    if (!isShowFollowingUp) return null

    return <Tracking proposal={data} />
  }

  renderSummary() {
    const { data, currentUserId } = this.props
    let isShowFollowingUp = _.includes(
      [CVOTE_STATUS.ACTIVE, CVOTE_STATUS.INCOMPLETED, CVOTE_STATUS.FINAL],
      data.status
    )

    if (
      data.status === CVOTE_STATUS.PROPOSED &&
      currentUserId === _.get(data, 'proposer._id')
    ) {
      isShowFollowingUp = true
    }
    if (!isShowFollowingUp) return null

    return <Summary proposal={data} />
  }

  gotoEditPage = () => {
    const { _id: id } = this.props.data
    this.props.history.push(`/proposals/${id}/edit`)
  }

  onNotesChanged = (e) => {
    this.setState({ notes: e.target.value })
  }

  showUpdateNotesModal = () => {
    const { notes } = this.props.data
    Modal.confirm({
      title: I18N.get('council.voting.modal.updateNotes'),
      content: <TextArea onChange={this.onNotesChanged} defaultValue={notes} />,
      okText: I18N.get('council.voting.modal.confirm'),
      cancelText: I18N.get('council.voting.modal.cancel'),
      onOk: () => this.updateNotes()
    })
  }

  async updateNotes() {
    const { notes } = this.state
    const id = _.get(this.props.match, 'params.id')

    if (_.isEmpty(notes)) return
    this.ord_loading(true)
    try {
      await this.props.updateNotes({
        _id: id,
        notes
      })
      message.success(I18N.get('from.CVoteForm.message.note.update.success'))
      this.refetch()
      this.ord_loading(false)
    } catch (error) {
      this.ord_loading(false)
      message.error(error.message)
      logger.error(error)
    }
  }

  renderVoteResults() {
    const {
      vote_map: voteMap,
      reason_map: reasonMap,
      voteResult,
      status
    } = this.props.data
    const { avatar_map: avatarMap } = this.props
    let stats
    if (status === CVOTE_STATUS.DRAFT) return null

    if (!_.isEmpty(voteResult)) {
      stats = _.reduce(
        voteResult,
        (prev, cur) => {
          const item = {
            name: `${_.get(cur, 'votedBy.profile.firstName')} ${_.get(
              cur,
              'votedBy.profile.lastName'
            )} `,
            didName: _.get(cur, 'votedBy.did.didName'),
            avatar: _.get(cur, 'votedBy.profile.avatar'),
            reason: cur.reason,
            votedBy: _.get(cur, 'votedBy._id'),
            status: _.get(cur, 'status')
          }
          if (prev[cur.value]) {
            prev[cur.value].push(item)
            return prev
          }
          return _.extend(prev, { [cur.value]: [item] })
        },
        {}
      )
    } else if (!_.isEmpty(voteMap)) {
      // for legacy data structure
      stats = _.reduce(
        voteMap,
        (prev, value, key) => {
          const item = {
            name: key,
            avatar: _.get(avatarMap, key),
            reason: _.get(reasonMap, key),
            status: key
          }
          if (prev[value]) {
            prev[value].push(item)
            return prev
          }
          return _.extend(prev, { [value]: [item] })
        },
        {}
      )
    }

    const {
      match,
      getReviewProposalUrl,
      getReviewProposal,
      isCouncil,
      data,
      currentUserId,
      updateProposal
    } = this.props
    const ownerVote = _.find(data.voteResult, (o) => {
      if (o.votedBy && o.votedBy._id == currentUserId) {
        return o
      }
    })
    const id = _.get(match, 'params.id')
    const isProposed = status == 'PROPOSED'
    const title = <h4>{I18N.get('council.voting.councilMembersVotes')}</h4>
    const detail = _.map(stats, (statArr, key) => {
      const type = CVOTE_RESULT[key.toUpperCase()] || CVOTE_RESULT.UNDECIDED
      const label = I18N.get(`council.voting.type.${type}`)
      const props = {
        dataList: statArr,
        type,
        label,
        id,
        getReviewProposal,
        getReviewProposalUrl,
        updateProposal,
        isProposed,
        isCouncil,
        currentUserId,
        ownerVote
      }
      return <VoteResultComponent {...props} key={key} />
    })
    return (
      <div id="vote">
        {title}
        <div>{detail}</div>
      </div>
    )
  }

  publish = async () => {
    const { match, updateCVote } = this.props
    const id = _.get(match, 'params.id')

    const param = { _id: id, published: true }

    this.ord_loading(true)
    try {
      await updateCVote(param)
      message.success(I18N.get('from.CVoteForm.message.updated.success'))
      this.refetch()
      this.ord_loading(false)
    } catch (e) {
      this.ord_loading(false)
      message.error(e.message)
      logger.error(e)
    }
  }

  deleteDraftProposal = async () => {
    const { match, deleteDraft } = this.props
    const id = _.get(match, 'params.id')
    const param = { _id: id }
    this.ord_loading(true)
    try {
      await deleteDraft(param)
      message.success(I18N.get('from.CVoteForm.message.delete.success'))
      // redirect to proposal list page
      this.props.history.push('/proposals')
      this.ord_loading(false)
    } catch (e) {
      this.ord_loading(false)
      message.error(e.message)
      logger.error(e)
    }
  }

  async vote({ value, reason }) {
    const { match, vote } = this.props
    const id = _.get(match, 'params.id')
    const param = { _id: id, value, reason }
    this.ord_loading(true)
    try {
      await vote(param)
      message.success(I18N.get('from.CVoteForm.message.updated.success'))
      this.refetch()
      this.ord_loading(false)
    } catch (e) {
      this.ord_loading(false)
      message.error(e.message)
    }
  }

  voteYes = ({ reason }) => {
    this.vote({ value: CVOTE_RESULT.SUPPORT, reason })
  }

  voteAbstain = ({ reason }) => {
    this.vote({ value: CVOTE_RESULT.ABSTENTION, reason })
  }

  voteOppose = ({ reason }) => {
    this.vote({ value: CVOTE_RESULT.REJECT, reason })
  }

  showVoteYesModal = () => {
    const { visibleYes, visibleOppose, visibleAbstain } = this.state
    this.setState({ visibleYes: !visibleYes })
    if (!visibleYes) {
      if (visibleOppose) this.setState({ visibleOppose: !visibleOppose })
      if (visibleAbstain) this.setState({ visibleAbstain: !visibleAbstain })
    }
  }

  showVoteAbstainModal = () => {
    const { visibleYes, visibleOppose, visibleAbstain } = this.state
    this.setState({ visibleAbstain: !visibleAbstain })
    if (!visibleAbstain) {
      if (visibleYes) this.setState({ visibleYes: !visibleYes })
      if (visibleOppose) this.setState({ visibleOppose: !visibleOppose })
    }
  }

  showVoteOpposeModal = () => {
    const { visibleYes, visibleOppose, visibleAbstain } = this.state
    this.setState({ visibleOppose: !visibleOppose })
    if (!visibleOppose) {
      if (visibleYes) this.setState({ visibleYes: !visibleYes })
      if (visibleAbstain) this.setState({ visibleAbstain: !visibleAbstain })
    }
  }

  completeProposal = () => {
    const title = I18N.get('council.voting.modal.complete')
    const fn = this.props.finishCVote
    this.finalizeProposal(title, fn)
  }

  incompleteProposal = () => {
    const title = I18N.get('council.voting.modal.incomplete')
    const fn = this.props.unfinishCVote
    this.finalizeProposal(title, fn)
  }

  finalizeProposal = (title, fn) => {
    const id = _.get(this.props.match, 'params.id')

    Modal.confirm({
      title,
      content: '',
      okText: I18N.get('council.voting.modal.confirm'),
      cancelText: I18N.get('council.voting.modal.cancel'),
      onOk: () => {
        this.ord_loading(true)
        fn({
          id
        })
          .then(() => {
            message.success(
              I18N.get('from.CVoteForm.message.proposal.update.success')
            )
            this.refetch()
            this.ord_loading(false)
          })
          .catch((e) => {
            this.ord_loading(false)
            message.error(e.message)
            logger.error(e)
          })
      }
    })
  }
}

export default C
