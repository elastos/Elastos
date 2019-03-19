import React from 'react';
import _ from 'lodash'
import { Row, Col, Spin } from 'antd'
import StandardPage from '../../StandardPage';
import Comments from '@/module/common/comments/Container'
import ActionsContainer from '../common/actions/Container'
import MetaContainer from '../common/meta/Container'
import Translation from '@/module/common/Translation/Container'
import MySuggestion from '../my_list/Container'
import Footer from '@/module/layout/Footer/Container'
import BackLink from "@/module/shared/BackLink/Component";

import I18N from '@/I18N'
import MediaQuery from 'react-responsive'
import { LG_WIDTH } from '@/config/constant'

import './style.scss'

export default class extends StandardPage {
  constructor(props) {
    super(props)

    // we use the props from the redux store if its retained
    this.state = {
      isDropdownActionOpen: false,
      showMobile: false,
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
    const mySuggestionNode = <MySuggestion />
    const commentNode = this.renderCommentNode()
    return (
      <div>
        <div className="p_SuggestionDetail">
          <BackLink link="/suggestion" />
          <MediaQuery maxWidth={LG_WIDTH}>
            <Row>
              <Col>
                {detailNode}
              </Col>
            </Row>
            <Row>
              <Col>
                {translationBtn}
              </Col>
            </Row>
            <Row>
              <Col>
                {actionsNode}
              </Col>
            </Row>
            <Row>
              <Col>
                {commentNode}
              </Col>
            </Row>
          </MediaQuery>
          <MediaQuery minWidth={LG_WIDTH + 1}>
            <Row gutter={24}>
              <Col span={15}>
                {detailNode}
                {translationBtn}
                {actionsNode}
                {commentNode}
              </Col>
              <Col span={9}>{mySuggestionNode}</Col>
            </Row>
          </MediaQuery>
        </div>
        <Footer />
      </div>
    )
  }

  renderDetail() {
    const metaNode = this.renderMetaNode()
    const titleNode = this.renderTitleNode()
    const descNode = this.renderDescNode()
    const linkNode = this.renderLinkNode()
    return (
      <div>
        {metaNode}
        {titleNode}
        {descNode}
        {linkNode}
      </div>
    )
  }

  renderMetaNode() {
    const { detail } = this.props
    return <MetaContainer data={detail} />
  }

  renderActionsNode() {
    const { detail } = this.props
    return <ActionsContainer data={detail} />
  }

  renderTitleNode() {
    const { detail } = this.props
    return (
      <div className="detail-title">{detail.title}</div>
    )
  }

  renderDescNode() {
    const { detail } = this.props
    return (
      <div className="detail-desc" dangerouslySetInnerHTML={{ __html: detail.desc }} />
    )
  }

  renderLinkNode() {
    const { detail } = this.props

    if (!detail.link) {
      return null
    }

    return (
      <div className="detail-link">
        {I18N.get('from.TaskCreateForm.label.info')}: <a href={detail.link} target="_blank">{detail.link}</a>
      </div>
    )
  }

  renderTranslationBtn() {
    const { title, desc } = this.props.detail
    const text = `<h1>${title}</h1>${desc}`

    return (
      <div style={{ marginTop: 20 }}>
        <Translation text={text} />
      </div>
    )
  }

  renderCommentNode() {
    const { detail } = this.props
    return (
      <Comments
        type="suggestion"
        suggestion={detail}
        canPost
        model={detail._id}
        returnUrl={`/suggestion/${detail._id}`}
      />
    )
  }

  showDropdownActions = () => {
    this.setState({
      isDropdownActionOpen: !this.state.isDropdownActionOpen,
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
