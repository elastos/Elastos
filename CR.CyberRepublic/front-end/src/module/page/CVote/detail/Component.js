import React from 'react'
import StandardPage from '../../StandardPage'
import {
  Form, Spin, Button, Row, Col, message, Modal,
} from 'antd'
import I18N from '@/I18N'
import _ from 'lodash'
import { LANGUAGES } from '@/config/constant'
import MetaComponent from '@/module/shared/Meta/Container'
import { CVOTE_RESULT_TEXT, CVOTE_TYPE } from '@/constant'

import './style.scss'


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
    }

    this.isLogin = this.props.isLogin
    this.user = this.props.user
  }

  componentDidMount() {
    this.refetch()
  }

  async refetch() {
    const data = await this.props.getData(this.props.match.params.id)
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
    const voteActionsNode = this.renderVoteActions()
    const voteDetailNode = this.renderVoteDetail()
    return (
      <div className="c_CVoteDetail">
        {metaNode}
        {titleNode}
        {subTitleNode}
        {contentNode}
        {voteActionsNode}
        {voteDetailNode}
      </div>
    )
  }

  async handleSubmit(e) {
    e.preventDefault()

    const s = this.props.static
    this.props.form.validateFields(async (err, values) => {
      if (!err) {
        // console.log(' ===> ', values)

        const param = {}
        const x1 = []
        const x2 = []
        const x3 = []
        _.each(s.voter, (n) => {
          const name = n.value
          x1.push(`${name}|${values[`vote_${name}`]}`)
          x2.push(`${name}|${values[`reason_${name}`]}`)
          x3.push(`${name}|${values[`reason_zh_${name}`]}`)
        })
        // vote_map: "Yipeng Su|undefined,Fay Li|undefined,Kevin Zhang|abstention"
        param.vote_map = x1.join(',')
        param.reason_map = x2.join(',')
        param.reason_zh_map = x3.join(',')

        // console.log(param)
        this.ord_loading(true)
        try {
          param._id = this.props.edit
          await this.props.updateCVote(param)
          message.success(I18N.get('from.CVoteForm.message.updated.success'))
          this.ord_loading(false)
          this.props.history.push('/proposals')
        } catch (e) {
          message.error(e.message)
          this.ord_loading(false)
        }
      }
    })
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

    // const voteObj = {
    //   text: `Online Voting by ${name}`,
    //   value: data.name,
    // }

    const dataList = [
      statusObj,
      publishObj,
      typeObj,
      // voteObj,
    ]
    return <SubTitle dataList={dataList} />
  }

  renderContent() {
    const { content } = this.state.data
    return <div className="content">{content}</div>
  }

  renderVoteActions() {
    if (!this.props.isCouncil) return null
    return (
      <div className="vote-btn-group">
        <Button
          type="primary"
          icon="check"
          onClick={this.voteYes}
        >
          {I18N.get('council.voting.btnText.yes')}
        </Button>
        <Button
          type="danger"
          icon="close"
          onClick={this.voteNo}
        >
          {I18N.get('council.voting.btnText.no')}
        </Button>
        <Button
          icon="delete"
          onClick={this.voteAbstention}
        >
          {I18N.get('council.voting.btnText.abstention')}
        </Button>
      </div>
    )
  }

  renderVoteDetail() {
    const { vote_map: voteMap } = this.state.data
    const stats = _.reduce(voteMap, (prev, value, key) => {
      if (prev[value]) {
        prev[value].push(key)
        return prev
      }
      return _.extend(prev, { [value]: [key] })
    }, {})
    const title = <h2>{I18N.get('council.voting.councilMembersVotes')}</h2>
    const detail = _.map(stats, (statArr, key) => {
      const avatars = _.map(statArr, (value, index) => <span key={index}>{value}</span>)
      return (
        <div key={key}>
          <span>{CVOTE_RESULT_TEXT[key]}</span>
          <span>{avatars}</span>
        </div>
      )
    })
    return (
      <div>
        {title}
        <div>{detail}</div>
      </div>
    )
  }

  async voteYes() {
    const { updateCVote } = this.props
    try {
      await updateCVote()
      message.success('Vote Successfully')
    } catch (error) {
      message.error(error.message)
    }
  }

  async voteNo() {
    const { updateCVote } = this.props
    try {
      await updateCVote()
      message.success('Vote Successfully')
    } catch (error) {
      message.error(error.message)
    }
  }

  async voteAbstention() {
    const { updateCVote } = this.props
    try {
      await updateCVote()
      message.success('Vote Successfully')
    } catch (error) {
      message.error(error.message)
    }
  }

  finishClick(id) {
    Modal.confirm({
      title: I18N.get('from.CVoteForm.modal.title'),
      content: '',
      okText: I18N.get('from.CVoteForm.modal.confirm'),
      okType: 'danger',
      cancelText: I18N.get('from.CVoteForm.modal.cancel'),
      onOk: () => {
        this.ord_loading(true)
        this.props.finishCVote({
          id,
        }).then(() => {
          message.success(I18N.get('from.CVoteForm.message.proposal.update.success'))
          this.ord_loading(false)
          this.props.history.push('/council')
        }).catch((e) => {
          message.error(e.message)
          this.ord_loading(false)
        })
      },
      onCancel() {
      },
    })
  }
}

export default Form.create()(C)
