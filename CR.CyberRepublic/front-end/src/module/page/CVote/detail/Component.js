import React from 'react'
import StandardPage from '../../StandardPage'
import {
  Form, Spin, Button, Input, message, Modal,
} from 'antd'
import I18N from '@/I18N'
import _ from 'lodash'
import { LANGUAGES } from '@/config/constant'
import { CVOTE_RESULT_TEXT, CVOTE_RESULT, CVOTE_TYPE, CVOTE_STATUS } from '@/constant'
import MetaComponent from '@/module/shared/meta/Container'
import VoteResultComponent from '../common/vote_result/Component'
import Footer from '@/module/layout/Footer/Container'

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
    }

    this.isLogin = this.props.isLogin
    this.user = this.props.user
  }

  componentDidMount() {
    this.refetch()
  }

  async refetch() {
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
    return (
      <div>
        <div className="p_CVoteDetail">
          {metaNode}
          {titleNode}
          {subTitleNode}
          {contentNode}
          {notesNode}
          {voteActionsNode}
          {adminActionsNode}
          {voteDetailNode}
        </div>
        <Footer />
      </div>
    )
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
      value: data.status || 'processing...',
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
    return <div className="content">{content}</div>
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
    const isExpired = status === CVOTE_STATUS.DEFERRED
    if (!isCouncil || isExpired) return null
    return (
      <div className="vote-btn-group">
        <Button
          type="primary"
          icon="check"
          onClick={this.showVoteYesModal}
        >
          {I18N.get('council.voting.btnText.yes')}
        </Button>
        <Button
          type="danger"
          icon="close"
          onClick={this.showVoteOpposeModal}
        >
          {I18N.get('council.voting.btnText.no')}
        </Button>
        <Button
          icon="delete"
          onClick={this.showVoteAbstentionModal}
        >
          {I18N.get('council.voting.btnText.abstention')}
        </Button>
      </div>
    )
  }

  renderAdminActions() {
    const { isSecretary, isCouncil } = this.props
    if (!isSecretary && !isCouncil) return null
    const { status } = this.state.data
    const isExpired = status === CVOTE_STATUS.DEFERRED
    const addNoteBtn = isSecretary && (
      <Button
        onClick={this.showUpdateNotesModal}
      >
        {I18N.get('council.voting.btnText.notesSecretary')}
      </Button>
    )
    const editProposalBtn = (isSecretary || isCouncil) && !isExpired && (
      <Button
        onClick={this.gotoEditPage}
      >
        {I18N.get('council.voting.btnText.editProposal')}
      </Button>
    )
    const completeProposalBtn = isSecretary && !isExpired && (
      <Button
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
    Modal.confirm({
      title: I18N.get('council.voting.modal.updateNotes'),
      content: <TextArea onChange={this.onNotesChanged} />,
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
    const { vote_map: voteMap, avatar_map: avatarMap } = this.state.data
    const stats = _.reduce(voteMap, (prev, value, key) => {
      const userObj = { name: key, avatar: avatarMap[key] }
      if (prev[value]) {
        prev[value].push(userObj)
        return prev
      }
      return _.extend(prev, { [value]: [userObj] })
    }, {})
    const title = <h2>{I18N.get('council.voting.councilMembersVotes')}</h2>
    const detail = _.map(stats, (statArr, key) => {
      const users = _.map(statArr, stat => ({ name: stat.name, avatar: stat.avatar }))
      const label = CVOTE_RESULT_TEXT[key]
      const type = label.toLowerCase()
      const props = {
        users,
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

  async vote({ vote, reason, reasonZh }) {
    const { data } = this.state
    const { match, updateCVote, currentUserId, static: { voter } } = this.props
    const id = _.get(match, 'params.id')

    const param = { _id: id }
    const voteMap = []
    const reasonMap = []
    const reasonMapZh = []
    _.each(voter, (name, voterId) => {
      if (voterId === currentUserId) {
        voteMap.push(`${name}|${vote}`)
        reasonMap.push(`${name}|${reason || data.reason_map[name]}`)
        reasonMapZh.push(`${name}|${reasonZh || data.reason_zh_map[name]}`)
      } else {
        voteMap.push(`${name}|${data.vote_map[name]}`)
        reasonMap.push(`${name}|${data.reason_map[name]}`)
        reasonMapZh.push(`${name}|${data.reason_zh_map[name]}`)
      }
    })
    param.vote_map = voteMap.join(',')
    if (reason) param.reason_map = reasonMap.join(',')
    if (reasonZh) param.reason_zh_map = reasonMapZh.join(',')

    this.ord_loading(true)
    try {
      await updateCVote(param)
      message.success(I18N.get('from.CVoteForm.message.updated.success'))
      this.refetch()
      this.ord_loading(false)
    } catch (e) {
      message.error(e.message)
      this.ord_loading(false)
    }
  }

  voteYes = () => {
    this.vote({ vote: CVOTE_RESULT.SUPPORT })
  }

  voteAbstention = () => {
    this.vote({ vote: CVOTE_RESULT.ABSTENTION })
  }

  voteOppose = ({ reason }) => {
    this.vote({ vote: CVOTE_RESULT.REJECT, reason })
    this.setState({ reason: '' })
  }

  onReasonChanged = (e) => {
    this.setState({ reason: e.target.value })
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
    const { reason } = this.state
    Modal.confirm({
      title: I18N.get('council.voting.modal.voteNo'),
      content: <TextArea onChange={this.onReasonChanged} />,
      okText: I18N.get('council.voting.modal.confirm'),
      cancelText: I18N.get('council.voting.modal.cancel'),
      onOk: () => this.voteOppose({ reason }),
    })
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
