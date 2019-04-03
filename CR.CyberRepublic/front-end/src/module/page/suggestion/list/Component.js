import React from 'react'
import { Link } from 'react-router-dom'
import _ from 'lodash'
import {
  Pagination, Modal, Button, Col, Row, Select, Spin,
} from 'antd'
import URI from 'urijs'
import I18N from '@/I18N'
import { loginRedirectWithQuery } from '@/util'
import StandardPage from '../../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import MySuggestion from '../my_list/Container'
import SuggestionForm from '@/module/form/SuggestionForm/Container'
import ActionsContainer from '../common/actions/Container'
import MetaContainer from '../common/meta/Container'

import MediaQuery from 'react-responsive'
import { MAX_WIDTH_MOBILE, MIN_WIDTH_PC, LG_WIDTH } from '@/config/constant'

import './style.scss'

const SORT_BY = {
  likesNum: 'likesNum',
  viewsNum: 'viewsNum',
  activeness: 'activeness',
  createdAt: 'createdAt',

}

/**
 * This uses new features such as infinite scroll and pagination, therefore
 * we do some different things such as only loading the data from the server
 */
export default class extends StandardPage {
  constructor(props) {
    super(props)

    const uri = URI(props.location.search || '')

    // we use the props from the redux store if its retained
    this.state = {
      showForm: uri.hasQuery('create'),
      isDropdownActionOpen: false,
      showMobile: false,
      page: 1,
      results: 10,
      total: 0,
    }
  }

  componentDidMount() {
    super.componentDidMount()
    this.refetch()
  }

  componentWillUnmount() {
    this.props.resetAll()
  }

  ord_renderContent() {
    const headerNode = this.renderHeader()
    const addButtonNode = this.renderAddButton()
    const actionsNode = this.renderHeaderActions()
    const mySuggestionNode = this.renderMySuggestion()
    const createForm = this.renderCreateForm()
    const listNode = this.renderList()

    return (
      <div>
        <div className="p_SuggestionList">
          {headerNode}
          <MediaQuery maxWidth={LG_WIDTH}>
            <Row>
              <Col>
                {addButtonNode}
                {mySuggestionNode}
              </Col>
            </Row>
            <Row>
              <Col>
                <br />
              </Col>
            </Row>
            <Row>
              <Col>
                {actionsNode}
                {listNode}
              </Col>
            </Row>
          </MediaQuery>
          <MediaQuery minWidth={LG_WIDTH + 1}>
            <Row gutter={24}>
              <Col span={16}>{actionsNode}</Col>
              <Col span={8}>{addButtonNode}</Col>
            </Row>
            <Row gutter={24}>
              <Col span={16}>
                {listNode}
              </Col>
              <Col span={8}>{mySuggestionNode}</Col>
            </Row>
          </MediaQuery>
          {createForm}
        </div>
        <Footer />
      </div>
    )
  }

  onFormSubmit = async (param) => {
    try {
      await this.props.create(param)
      this.showCreateForm()
      this.refetch()
    } catch (error) {
      // console.log(error)
    }
  }

  renderCreateForm = () => {
    const props = {
      onFormCancel: this.showCreateForm,
      onFormSubmit: this.onFormSubmit,
    }

    return (
      <Modal
        className="project-detail-nobar"
        maskClosable={false}
        visible={this.state.showForm}
        onOk={this.showCreateForm}
        onCancel={this.showCreateForm}
        footer={null}
        width="70%"
      >
        { this.state.showForm
          && <SuggestionForm {...props} />
        }
      </Modal>
    )
  }

  showCreateForm = () => {
    const { isLogin, history } = this.props
    const { showForm } = this.state
    if (!isLogin) {
      const query = { create: true }
      loginRedirectWithQuery({ query })
      history.push('/login')
      return
    }
    this.setState({
      showForm: !showForm,
    })
  }

  renderHeader() {
    return (
      <div>
        <h2 className="title komu-a cr-title-with-icon">{this.props.header || I18N.get('suggestion.title').toUpperCase()}</h2>

        <p style={{width: '60%', paddingBottom: '60px'}}>
          {I18N.get('suggestion.intro.1')}<Link to="/proposals">{I18N.get('suggestion.intro.1.proposals')}</Link>{I18N.get('suggestion.intro.1.1')}
          <br/>
          <br/>
          {I18N.get('suggestion.intro.3')}{localStorage.getItem('lang') === 'en' ?
          <a href="https://www.cyberrepublic.org/docs/#/guide/suggestions" target="_blank">https://www.cyberrepublic.org/docs/#/guide/suggestions</a> :
          <a href="https://www.cyberrepublic.org/docs/#/zh/guide/suggestions" target="_blank">https://www.cyberrepublic.org/docs/#/zh/guide/suggestions</a>
          }
          <br/>
          <br/>
          {I18N.get('suggestion.intro.2')}{localStorage.getItem('lang') === 'en' ?
          <a target="_blank" href="https://blog.cyberrepublic.org/2019/01/23/crc-suggestions-launch-empower35-crcles-update">{I18N.get('suggestion.intro.2.blog')}</a> :
          <a target="_blank" href="https://blog.cyberrepublic.org/zh/2019/01/23/%E5%85%B3%E4%BA%8Ecr%E5%85%B1%E8%AF%86%E5%AE%9A%E4%B9%89%EF%BC%8C%E4%B8%8A%E7%BA%BF%E5%BB%BA%E8%AE%AE%E9%A1%B5%E9%9D%A2%E4%BB%A5%E5%8F%8Aempower35-crcles%E7%9A%84%E6%9B%B4%E6%96%B0/">{I18N.get('suggestion.intro.2.blog')}</a>
          }
        </p>
      </div>
    )
  }

  renderHeaderActions() {
    const SORT_BY_TEXT = {
      likesNum: I18N.get('suggestion.likes'),
      viewsNum: I18N.get('suggestion.views'),
      activeness: I18N.get('suggestion.activeness'),
      createdAt: I18N.get('suggestion.dateAdded'),
    }
    const sortBy = this.props.sortBy || SORT_BY.likesNum
    return (
      <div className="header-actions-container">
        <MediaQuery maxWidth={LG_WIDTH}>
          <Select
            name="type"
            onChange={this.onSortByChanged}
            value={sortBy}
          >
            {_.map(SORT_BY, value => (
              <Select.Option key={value} value={value}>
                {SORT_BY_TEXT[value]}
              </Select.Option>
            ))}
          </Select>
        </MediaQuery>
        <MediaQuery minWidth={LG_WIDTH + 1}>
          <Button.Group className="filter-group">
            {_.map(SORT_BY, value => (
              <Button
                key={value}
                onClick={() => this.onSortByChanged(value)}
                className={(sortBy === value && 'cr-strikethrough') || ''}
              >
                {SORT_BY_TEXT[value]}
              </Button>
            ))}
          </Button.Group>
        </MediaQuery>
      </div>
    )
  }

  renderAddButton() {
    return (
      <div className="pull-left filter-group btn-create-suggestion">
        <Button onClick={this.showCreateForm}>
          {I18N.get('suggestion.add')}
        </Button>
      </div>
    )
  }

  renderList() {
    const { dataList, loading } = this.props
    const loadingNode = <div className="center"><Spin size="large" /></div>
    const paginationNode = this.renderPagination()
    let result = loadingNode
    if (!loading) {
      if (_.isEmpty(dataList)) {
        result = <div className="center">{I18N.get('suggestion.nodata')}</div>
      } else {
        result = _.map(dataList, data => this.renderItem(data))
      }
    }

    return (
      <div>
        <div className="list-container">
          <div>
            <h2 className="title komu-a">{I18N.get('suggestion.listTitle').toUpperCase()}</h2>
          </div>
          {result}
        </div>
        {paginationNode}
      </div>
    )
  }

  renderItem = (data) => {
    const href = `/suggestion/${data._id}`
    const actionsNode = this.renderActionsNode(data)
    const metaNode = this.renderMetaNode(data)
    const title = <Link to={href} className="title-link">{data.title}</Link>
    return (
      <div key={data._id} className="item-container">
        {metaNode}
        {title}
        {actionsNode}
      </div>
    )
  }

  renderPagination() {
    const { total } = this.props
    const { results, page } = this.state
    const props = {
      pageSize: results,
      total,
      current: page,
      onChange: this.loadPage,
    }
    return <Pagination {...props} className="cr-pagination" />
  }

  renderMetaNode = detail => <MetaContainer data={detail} />

  renderActionsNode = detail => <ActionsContainer data={detail} />

  renderMySuggestion = () => <MySuggestion />

  onSortByChanged = async (sortBy) => {
    await this.props.onSortByChanged(sortBy)
    await this.refetch()
  }

  /**
   * Builds the query from the current state
   */
  getQuery = () => {
    const sortBy = this.props.sortBy || SORT_BY.likesNum
    const { page, results } = this.state
    const query = {
      page,
      results,
    }
    // TODO
    if (sortBy) {
      query.sortBy = sortBy
    }

    return query
  }

  /**
   * Refetch the data based on the current state retrieved from getQuery
   */
  refetch = () => {
    const query = this.getQuery()
    this.props.getList(query)
  }

  loadPage = async (page) => {
    const query = {
      ...this.getQuery(),
      page,
      results: this.state.results,
    }

    this.setState({ loadingMore: true })

    try {
      await this.props.loadMore(query)
      this.setState({ page })
    } catch (e) {
      // Do not update page in state if the call fails
    }

    this.setState({ loadingMore: false })
  }

  gotoDetail(id) {
    this.props.history.push(`/suggestion/${id}`)
  }
}
