import React from 'react';
import _ from 'lodash'
import {
  Pagination, Modal, Button, Col, Row, Select, Spin,
} from 'antd';
import I18N from '@/I18N'
import StandardPage from '../../StandardPage';
import Footer from '@/module/layout/Footer/Container'
import MySuggestion from '../my_list/Container'
import SuggestionForm from '@/module/form/SuggestionForm/Container'
import ActionsContainer from '../common/actions/Container'
import MetaContainer from '../common/meta/Container'

import MediaQuery from 'react-responsive'
import { MAX_WIDTH_MOBILE, MIN_WIDTH_PC } from '@/config/constant'

import './style.scss'

const SORT_BY = {
  likesNum: 'likesNum',
  viewsNum: 'viewsNum',
  activeness: 'activeness',
  createdAt: 'createdAt',

}

const SORT_BY_TEXT = {
  likesNum: I18N.get('suggestion.likes'),
  viewsNum: I18N.get('suggestion.views'),
  activeness: I18N.get('suggestion.activeness'),
  createdAt: I18N.get('suggestion.dateAdded'),
}

/**
 * This uses new features such as infinite scroll and pagination, therefore
 * we do some different things such as only loading the data from the server
 */
export default class extends StandardPage {
  constructor(props) {
    super(props)

    // we use the props from the redux store if its retained
    this.state = {
      showForm: false,
      isDropdownActionOpen: false,
      showMobile: false,
      sortBy: SORT_BY.likesNum,
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
          <Row gutter={24}>
            <Col span={15}>{actionsNode}</Col>
            <Col span={9}>{addButtonNode}</Col>
          </Row>
          <Row gutter={24}>
            <Col span={15}>
              {listNode}
            </Col>
            <Col span={9}>{mySuggestionNode}</Col>
          </Row>
          {createForm}
        </div>
        <Footer />
      </div>
    )
  }

  renderCreateForm() {
    return (
      <Modal
        className="project-detail-nobar"
        visible={this.state.showForm}
        onOk={this.showCreateForm}
        onCancel={this.showCreateForm}
        footer={null}
        width="70%"
      >
        { this.state.showForm
          && <SuggestionForm showCreateForm={this.showCreateForm} refetch={this.refetch} />
        }
      </Modal>
    )
  }

  showCreateForm = () => {
    this.setState({
      showForm: !this.state.showForm,
    })
  }

  renderHeader() {
    return (
      <h2 className="title komu-a cr-title-with-icon">{this.props.header || I18N.get('suggestion.title').toUpperCase()}</h2>
    )
  }

  renderHeaderActions() {
    return (
      <div className="header-actions-container">
        <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
          <Select
            name="type"
            onChange={this.onSortByChanged}
            value={this.state.sortBy}
          >
            {_.map(SORT_BY, value => (
              <Select.Option key={value} value={value}>
                {SORT_BY_TEXT[value]}
              </Select.Option>
            ))}
          </Select>
        </MediaQuery>
        <MediaQuery minWidth={MIN_WIDTH_PC}>
          <Button.Group className="filter-group">
            {_.map(SORT_BY, value => (
              <Button
                key={value}
                onClick={() => this.onSortByChanged(value)}
                className={(this.state.sortBy === value && 'cr-strikethrough') || ''}
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
    const { dataList, loading } = this.props;
    const loadingNode = <div className="center"><Spin size="large" /></div>
    const paginationNode = this.renderPagination()
    let result = loadingNode
    if (!loading) {
      if (_.isEmpty(dataList)) {
        result = <div className="center">{I18N.get('suggestion.befirst')}</div>
      } else {
        result = _.map(dataList, data => this.renderItem(data))
      }
    }

    return (
      <div>
        <div className="list-container">{result}</div>
        {paginationNode}
      </div>
    )
  }

  renderItem = (data) => {
    const href = `/suggestion/${data._id}`
    const actionsNode = this.renderActionsNode(data)
    const metaNode = this.renderMetaNode(data)
    const title = <a href={href} className="title-link">{data.title}</a>
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

  onSortByChanged = sortBy => this.setState({ sortBy }, this.refetch)

  /**
   * Builds the query from the current state
   */
  getQuery = () => {
    const { page, results } = this.state
    const query = {
      page,
      results,
    }
    // TODO
    if (this.state.sortBy) {
      query.sortBy = this.state.sortBy
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
