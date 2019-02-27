import React from 'react'
import StandardPage from '../../StandardPage'
import {
  Form, Spin, Button, Input, message, Modal, Icon,
} from 'antd'
import I18N from '@/I18N'
import _ from 'lodash'
import { LANGUAGES } from '@/config/constant'
import { CVOTE_RESULT_TEXT, CVOTE_RESULT, CVOTE_TYPE, CVOTE_STATUS, CVOTE_STATUS_TEXT } from '@/constant'
import MetaComponent from '@/module/shared/meta/Container'
import VoteResultComponent from '../common/vote_result/Component'
import EditForm from '../edit/Container'
import Footer from '@/module/layout/Footer/Container'
import BackLink from "@/module/shared/BackLink/Component";
import CRPopover from "@/module/shared/Popover/Component";

import './style.scss'

const { TextArea } = Input;

const SubTitle = ({ dataList }) => {
  const result = _.map(dataList, (data, key) => (
    <h4 className="subtitle-item" key={key}>
      <div className="text">{data.text}</div>
      <div className="value">{data.value}</div>
    </h4>
  ))
  return <div className="subtitle-container">{result}</div>
}

class C extends StandardPage {
  constructor(props) {
    super(props)

    this.state = {
      persist: true,
      loading: false,
      language: LANGUAGES.english, // language for this specifc form only
      data: undefined,
      reason: '',
      visible: false,
      editing: false,
    }

    this.isLogin = this.props.isLogin
    this.user = this.props.user
  }

  componentDidMount() {
    this.refetch()
  }

  refetch = async () => {
    const data = await this.props.getData(_.get(this.props.match, 'params.id'))
    this.setState({ data })
  }

  ord_loading(f = false) {
    this.setState({ loading: f })
  }

  ord_renderContent() {
    if (!this.state.data) {
      return <div className="center"><Spin /></div>
    }
    const metaNode = this.renderMeta()
    const titleNode = this.renderTitle()
    const subTitleNode = this.renderSubTitle()
    const contentNode = this.renderContent()
    const notesNode = this.renderNotes()
    const voteActionsNode = this.renderVoteActions()
    const adminActionsNode = this.renderAdminActions()
    const voteDetailNode = this.renderVoteResults()
    const editFormNode = this.renderEditForm()
    return (
      <div>
        <div className="p_CVoteDetail">
          <BackLink link="/proposals" />
          {metaNode}
          {titleNode}
          {subTitleNode}
          {contentNode}
          {notesNode}
          {voteActionsNode}
          {adminActionsNode}
          {voteDetailNode}
          {editFormNode}
        </div>
        <Footer />
      </div>
    )
  }

  renderEditForm() {
    return (
      <Modal
        className="project-detail-nobar"
        visible={this.state.editing}
        onOk={this.switchEditMode}
        onCancel={this.switchEditMode}
        footer={null}
        width="70%"
      >
        <EditForm onEdit={this.onEdit} onCancel={this.switchEditMode} />
      </Modal>
    )
  }

  switchEditMode = () => {
    const { editing } = this.state
    this.setState({
      editing: !editing,
    })
  }

  onEdit = () => {
    this.switchEditMode()
    this.refetch()
  }

  renderMeta() {
    const { data } = this.state
    data.author = data.proposedBy
    data.displayId = data.vid
    const postedByText = I18N.get('from.CVoteForm.label.proposedby')
    return <MetaComponent data={data} postedByText={postedByText} />
  }

  renderTitle() {
    const { title } = this.state.data
    return <h2>{title}</h2>
  }

  renderSubTitle() {
    const { data } = this.state
    const statusObj = {
      text: I18N.get('from.CVoteForm.label.voteStatus'),
      value: CVOTE_STATUS_TEXT[data.status] || 'processing...',
    }

    const publishObj = {
      text: I18N.get('from.CVoteForm.label.publish'),
      value: data.published ? 'Yes' : 'No',
    }

    const typeObj = {
      text: I18N.get('from.CVoteForm.label.type'),
      value: CVOTE_TYPE[data.type],
    }

    const voteObj = {
      text: I18N.get('council.voting.ifConflicted'),
      value: data.isConflict === 'YES' ? 'Yes' : 'No',
    }

    const dataList = [
      statusObj,
      publishObj,
      typeObj,
      voteObj,
    ]
    return <SubTitle dataList={dataList} />
  }

  renderContent() {
    const { content } = this.state.data
    return <div className="content" dangerouslySetInnerHTML={{ __html: content }} />
  }

  renderNotes() {
    const { notes, notes_zh } = this.state.data
    if (!notes && !notes_zh) return null
    return (
      <div className="content notes">
        <h4 className="notes-title">{I18N.get('council.voting.btnText.notesSecretary')}</h4>
        <p className="notes-content">{notes}</p>
        <p className="notes-content">{notes_zh}</p>
      </div>
    )
  }

  renderVoteActions() {
    const { isCouncil } = this.props
    const { status } = this.state.data
    const canVote = isCouncil && status === CVOTE_STATUS.PROPOSED

    if (!canVote) return null
    const { visible } = this.state
    const opposeBtn = (
      <Button
        type="danger"
        icon="close-circle"
        onClick={this.showVoteOpposeModal}
      >
        {I18N.get('council.voting.btnText.no')}
      </Button>
    )

    const opposePopOver = (
      <CRPopover
        triggeredBy={opposeBtn}
        visible={visible}
        onToggle={this.showVoteOpposeModal}
        onSubmit={this.voteOppose}
      />
    )
    return (
      <div className="vote-btn-group">
        <Button
          type="primary"
          icon="check-circle"
          onClick={this.showVoteYesModal}
        >
          {I18N.get('council.voting.btnText.yes')}
        </Button>
        {opposePopOver}
        <Button
          icon="stop"
          onClick={this.showVoteAbstentionModal}
        >
          {I18N.get('council.voting.btnText.abstention')}
        </Button>
      </div>
    )
  }

  renderAdminActions() {
    const { isSecretary, isCouncil, currentUserId } = this.props
    const { status, createdBy, notes } = this.state.data
    const isSelf = currentUserId === createdBy
    const isCompleted = status === CVOTE_STATUS.FINAL
    const canManage = isSecretary || isCouncil
    const canEdit = _.includes([CVOTE_STATUS.DRAFT, CVOTE_STATUS.PROPOSED], status)
    const canComplete = _.includes([CVOTE_STATUS.ACTIVE, CVOTE_STATUS.REJECT, CVOTE_STATUS.DEFERRED], status)

    if (!canManage || isCompleted) return null

    const noteBtnText = notes ? I18N.get('council.voting.btnText.editNotes') : I18N.get('council.voting.btnText.notesSecretary')
    const addNoteBtn = isSecretary && (
      <Button
        icon="profile"
        onClick={this.showUpdateNotesModal}
      >
        {noteBtnText}
      </Button>
    )
    const editProposalBtn = isSelf && canEdit && (
      <Button
        icon="edit"
        onClick={this.switchEditMode}
      >
        {I18N.get('council.voting.btnText.editProposal')}
      </Button>
    )
    const completeProposalBtn = isSecretary && canComplete && (
      <Button
        icon="check-square"
        type="primary"
        onClick={this.completeProposal}
      >
        {I18N.get('council.voting.btnText.completeProposal')}
      </Button>
    )
    return (
      <div className="vote-btn-group">
        {addNoteBtn}
        {editProposalBtn}
        {completeProposalBtn}
      </div>
    )
  }

  onNotesChanged = (e) => {
    this.setState({ notes: e.target.value })
  }

  showUpdateNotesModal = () => {
    const { notes } = this.state.data
    Modal.confirm({
      title: I18N.get('council.voting.modal.updateNotes'),
      content: <TextArea onChange={this.onNotesChanged} defaultValue={notes} />,
      okText: I18N.get('council.voting.modal.confirm'),
      cancelText: I18N.get('council.voting.modal.cancel'),
      onOk: () => this.updateNotes(),
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
        notes,
      })
      message.success(I18N.get('from.CVoteForm.message.note.update.success'))
      this.refetch()
      this.ord_loading(false)
    } catch (error) {
      message.error(error.message)
      this.ord_loading(false)
    }
  }

  gotoEditPage = () => {
    const { match, history } = this.props
    const id = _.get(match, 'params.id')
    history.push(`/cvote/edit/${id}`)
  }

  renderVoteResults() {
    const { vote_map: voteMap, reason_map: reasonMap, voteResult, status } = this.state.data
    const { avatar_map: avatarMap } = this.props
    let stats

    if (status === CVOTE_STATUS.DRAFT) return null

    if (!_.isEmpty(voteResult)) {
      stats = _.reduce(voteResult, (prev, cur) => {
        const item = {
          name: `${_.get(cur, 'votedBy.profile.firstName')} ${_.get(cur, 'votedBy.profile.lastName')} `,
          avatar: _.get(cur, 'votedBy.profile.avatar'),
          reason: cur.reason,
        }
        if (prev[cur.value]) {
          prev[cur.value].push(item)
          return prev
        }
        return _.extend(prev, { [cur.value]: [item] })
      }, {})
    } else if (!_.isEmpty(voteMap)) {
      // for legacy data structure
      stats = _.reduce(voteMap, (prev, value, key) => {
        const item = { name: key, avatar: _.get(avatarMap, key), reason: _.get(reasonMap, key) }
        if (prev[value]) {
          prev[value].push(item)
          return prev
        }
        return _.extend(prev, { [value]: [item] })
      }, {})
    }

    const title = <h2>{I18N.get('council.voting.councilMembersVotes')}</h2>
    const detail = _.map(stats, (statArr, key) => {
      const label = CVOTE_RESULT_TEXT[key]
      const type = label.toLowerCase()
      const props = {
        dataList: statArr,
        type,
        label,
      }
      return <VoteResultComponent {...props} key={key} />
    })
    return (
      <div>
        {title}
        <div>{detail}</div>
      </div>
    )
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
      message.error(e.message)
      this.ord_loading(false)
    }
  }

  voteYes = () => {
    this.vote({ value: CVOTE_RESULT.SUPPORT })
  }

  voteAbstention = () => {
    this.vote({ value: CVOTE_RESULT.ABSTENTION })
  }

  voteOppose = ({ reason }) => {
    this.vote({ value: CVOTE_RESULT.REJECT, reason })
    this.setState({ reason: '' })
  }

  showVoteYesModal = () => {
    Modal.confirm({
      title: I18N.get('council.voting.modal.voteYes'),
      okText: I18N.get('council.voting.modal.confirm'),
      cancelText: I18N.get('council.voting.modal.cancel'),
      onOk: () => this.voteYes(),
    })
  }

  showVoteAbstentionModal = () => {
    Modal.confirm({
      title: I18N.get('council.voting.modal.voteAbstention'),
      okText: I18N.get('council.voting.modal.confirm'),
      cancelText: I18N.get('council.voting.modal.cancel'),
      onOk: () => this.voteAbstention(),
    })
  }

  showVoteOpposeModal = () => {
    const { visible } = this.state
    this.setState({ visible: !visible })
  }

  completeProposal = () => {
    const id = _.get(this.props.match, 'params.id')

    Modal.confirm({
      title: I18N.get('council.voting.modal.complete'),
      content: '',
      okText: I18N.get('council.voting.modal.confirm'),
      cancelText: I18N.get('council.voting.modal.cancel'),
      onOk: () => {
        this.ord_loading(true)
        this.props.finishCVote({
          id,
        }).then(() => {
          message.success(I18N.get('from.CVoteForm.message.proposal.update.success'))
          this.refetch()
          this.ord_loading(false)
        }).catch((e) => {
          message.error(e.message)
          this.ord_loading(false)
        })
      },
    })
  }
}

export default Form.create()(C)
