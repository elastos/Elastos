import React from 'react'
import _ from 'lodash'
import MediaQuery from 'react-responsive'
import {
  Col, Row, Input, Select, Button, Table,
} from 'antd'
import moment from 'moment/moment'
import I18N from '@/I18N'
import AdminPage from '../../BaseAdmin'
import Footer from '@/module/layout/Footer/Container'
import Navigator from '@/module/page/shared/HomeNavigator/Container'
import {MAX_WIDTH_MOBILE, MIN_WIDTH_PC} from '@/config/constant'
import {SUGGESTION_STATUS} from '@/constant'

import {ReactComponent as ArchiveIcon} from '@/assets/images/icon-archive.svg'
import {ReactComponent as DeleteIcon} from '@/assets/images/icon-delete.svg'
import {ReactComponent as AbuseIcon} from '@/assets/images/icon-abuse.svg'

import '@/module/page/admin/admin.scss'
import './style.scss'

const FILTERS = {
  ALL: 'all',
  ARCHIVED: SUGGESTION_STATUS.ARCHIVED,
}

export default class extends AdminPage {
  constructor(props) {
    super(props)

    this.state = {
      page: 1,
      results: 10,
      showMobile: false,
      filter: FILTERS.ALL,
      search: '',
    }
    this.debouncedLoadMore = _.debounce(this.loadMore.bind(this), 300)
    this.debouncedRefetch = _.debounce(this.refetch.bind(this), 300)
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
    const actionsNode = this.renderHeaderActions()
    const listNode = this.renderList()
    return (
      <div>
        <div className="p_AdminSuggestionList ebp-wrap">
          <Row>
            <Col sm={24} md={4} className="wrap-box-navigator">
              <Navigator selectedItem="profileAdminSuggestions"/>
            </Col>
            <Col sm={24} md={20} className="c_ProfileContainer admin-right-column wrap-box-user">
              {headerNode}
              {actionsNode}
              {listNode}
            </Col>
          </Row>
        </div>
        <Footer/>
      </div>
    )
  }

  renderHeader() {
    return (
      <h2
        className="title komu-a cr-title-with-icon">
        {this.props.header || I18N.get('suggestion.title').toUpperCase()}
      </h2>
    )
  }

  onSearchChanged = (e) => {
    const search = e.target.value
    this.setState({
      search,
      page: 1,
    }, this.debouncedRefetch)
  }

  renderSearch() {
    return (
      <div className="pull-left filter-group search-group">
        <Input
          defaultValue={this.state.search}
          onChange={this.onSearchChanged}
          placeholder={I18N.get('developer.search.search.placeholder')}
        />
      </div>
    )
  }

  renderAdminActions() {
    const {archive, remove, abuse} = this.props
    return (
      <span className="admin-actions">
        <span onClick={() => this.updateAndRefetch(archive)}>{<ArchiveIcon/>}</span>
        <span onClick={() => this.updateAndRefetch(remove)}>{<DeleteIcon/>}</span>
        <span onClick={() => this.updateAndRefetch(abuse)}>{<AbuseIcon/>}</span>
      </span>
    )
  }

  updateAndRefetch = async (actionFn) => {
    await actionFn(this.state.currRowId)
    this.refetch()
  }

  renderColumns() {
    const columns = [
      {
        title: <span>#</span>,
        dataIndex: 'displayId',
        sorter: (a, b) => a.displayId - b.displayId,
        defaultSortOrder: 'descend',
      },
      {
        title: I18N.get('suggestion.subject'),
        dataIndex: 'title',
        // width: '50%',
        className: 'fontWeight500 allow-wrap',
        render: (title, data) => (
          <a onClick={() => this.linkSuggestionDetail(data._id)}
            className="tableLink">
            {title}
          </a>
        ),
      },
      {
        title: <span>{I18N.get('suggestion.likes')}</span>,
        dataIndex: 'likesNum',
        sorter: (a, b) => a.likesNum - b.likesNum,
      },
      {
        title: <span>{I18N.get('suggestion.dislikes')}</span>,
        dataIndex: 'dislikesNum',
        sorter: (a, b) => a.dislikesNum - b.dislikesNum,
      },
      {
        title: <span>{I18N.get('suggestion.comments')}</span>,
        dataIndex: 'commentsNum',
        sorter: (a, b) => a.commentsNum - b.commentsNum,
      },
      {
        title: <span>{I18N.get('suggestion.activeness')}</span>,
        dataIndex: 'activeness',
        sorter: (a, b) => a.activeness - b.activeness,
      },
      {
        title: I18N.get('suggestion.owner'),
        dataIndex: 'createdBy.profile',
        render: data => `${_.get(data, 'firstName')} ${_.get(data, 'lastName')}`,
      },
      {
        title: <span>{I18N.get('suggestion.created')}</span>,
        dataIndex: 'createdAt',
        render: createdAt => moment(createdAt).format('MMM D'),
        sorter: (a, b) => moment(a.createdAt).valueOf() - moment(b.createdAt).valueOf(),
      },
      {
        title: '',
        dataIndex: '_id',
        render: _id => (_id === this.state.currRowId ? this.renderAdminActions() : ''),
      },
    ]
    return columns
  }

  renderHeaderActions() {
    const searchNode = this.renderSearch()
    return (
      <div className="header-actions-container">
        {searchNode}
        <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
          <Select
            name="type"
            onChange={this.onFilterChanged}
            value={this.state.filter}
          >
            {_.map(FILTERS, (filter, key) => (
              <Select.Option key={filter} value={filter}>
                {key}
              </Select.Option>
            ))}
          </Select>
        </MediaQuery>
        <MediaQuery minWidth={MIN_WIDTH_PC}>
          <Button.Group className="filter-group">
            <Button
              className={(this.state.filter === FILTERS.ALL && 'selected') || ''}
              onClick={this.clearFilters}
            >
              {I18N.get('suggestion.all')}
            </Button>
            <Button
              className={(this.state.filter === FILTERS.ARCHIVED && 'selected') || ''}
              onClick={this.setArchivedFilter}
            >
              {I18N.get('suggestion.archived')}
            </Button>
          </Button.Group>
        </MediaQuery>
      </div>
    )
  }

  renderList() {
    const {all_suggestions: dataList, loading, all_suggestions_total: total} = this.props
    const columns = this.renderColumns()

    return (
      <Table
        columns={columns}
        onRow={record => ({onMouseEnter: event => this.setState({currRowId: record._id})})}
        rowKey={item => item._id}
        dataSource={dataList}
        loading={loading}
        onChange={this.onTableChanged}
        pagination={{
          pageSize: this.state.results,
          total: loading ? 0 : total,
          onChange: this.loadPage,
        }}
      />
    )
  }

  onTableChanged = (pagination, filters, sorter) => {
    this.setState({
      filteredInfo: filters,
      pagination,
    })
  }

  onSortByChanged = (sortBy) => {
    this.setState({
      sortBy,
    }, this.refetch)
  }

  /**
   * Builds the query from the current state
   */
  getQuery = () => {
    const query = {}

    if (this.state.sortBy) {
      query.sortBy = this.state.sortBy
    }

    if (!_.isEmpty(this.state.search)) {
      query.search = this.state.search
    }

    if (this.state.filter === FILTERS.ARCHIVED) {
      query.status = FILTERS.ARCHIVED
    }

    query.page = this.state.page
    query.results = this.state.results

    return query
  }

  /**
   * Refetch the data based on the current state retrieved from getQuery
   */
  refetch = () => {
    const query = this.getQuery()
    this.props.getSuggestions(query)
  }

  loadMore = async () => {
    const page = this.state.page + 1

    const query = {
      ...this.getQuery(),
      page,
      results: this.state.results,
    }

    this.setState({loadingMore: true})

    try {
      await this.props.loadMoreSuggestions(query)
      this.setState({page})
    } catch (e) {
      // Do not update page in state if the call fails
    }

    this.setState({loadingMore: false})
  }

  loadPage = async (page) => {
    const query = {
      ...this.getQuery(),
      page,
      results: this.state.results,
    }

    this.setState({loadingMore: true})

    try {
      await this.props.loadMoreSuggestions(query)
      this.setState({page})
    } catch (e) {
      // Do not update page in state if the call fails
    }

    this.setState({loadingMore: false})
  }

  hasMoreSuggestions = () => _.size(this.props.all_suggestions) < this.props.all_suggestions_total

  onFilterChanged = (value) => {
    switch (value) {
      case FILTERS.ALL:
        this.clearFilters()
        break
      case FILTERS.ARCHIVED:
        this.setArchivedFilter()
        break
      default:
        this.clearFilters()
        break
    }
  }

  clearFilters = () => {
    this.setState({filter: FILTERS.ALL}, this.refetch)
  }

  setArchivedFilter = () => {
    this.setState({filter: FILTERS.ARCHIVED}, this.refetch)
  }

  linkSuggestionDetail = (suggestionId) => {
    this.props.history.push(`/suggestion/${suggestionId}`)
  }
}
