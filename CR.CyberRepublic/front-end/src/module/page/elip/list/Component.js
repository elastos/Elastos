import React from 'react'
import _ from 'lodash'
import moment from 'moment/moment'
import BaseComponent from '@/model/BaseComponent'
import { Table, Row, Col, Button, Select, Input, DatePicker } from 'antd'
import { CSVLink } from 'react-csv'
import I18N from '@/I18N'
import { ELIP_STATUS, ELIP_TYPE } from '@/constant'
import rangePickerLocale from 'antd/es/date-picker/locale/zh_CN'
import {
  Container,
  StyledButton,
  StyledSearch,
  FilterLabel,
  FilterPanel,
  FilterContent,
  FilterItem,
  FilterItemLabel,
  FilterClearBtn
} from './style'
import { logger } from '@/util'
import userUtil from '@/util/user'
import { ReactComponent as UpIcon } from '@/assets/images/icon-up.svg'
import { ReactComponent as DownIcon } from '@/assets/images/icon-down.svg'


const SORT_BY = {
  createdAt: 'createdAt',
  updatedAt: 'updatedAt'
}
const DEFAULT_SORT = SORT_BY.createdAt

const { RangePicker } = DatePicker

export default class extends BaseComponent {
  constructor(props) {
    super(props)

    const { isVisitableFilter } = this.props
    const {
      search,
      filter,
      creationDate,
      author,
      type
    } = this.props.filters
    this.state = {
      list: [],
      loading: true,
      isVisitableFilter,
      search,
      filter,
      creationDate,
      author,
      type,
    }
    this.debouncedRefetch = _.debounce(this.refetch.bind(this), 300)
  }

  async componentDidMount() {
    this.refetch()
  }

  getQuery = () => {
    const sortBy = this.props.sortBy || DEFAULT_SORT
    const { filter, creationDate, author, type } = this.state
    const query = {}
    if (!_.isEmpty(filter)) {
      query.filter = filter
    }
    if (!_.isEmpty(creationDate)) {
      const formatStr = 'YYYY-MM-DD'
      query.startDate = moment(creationDate[0]).format(formatStr)
      query.endDate = moment(creationDate[1]).format(formatStr)
    }
    if (!_.isEmpty(author)) {
      query.author = author
    }
    if (!_.isEmpty(type)) {
      query.type = type
    }
    if (sortBy) {
      query.sortBy = sortBy
    }
    const searchStr = this.state.search
    if (searchStr) {
      query.search = searchStr
    }
    return query
  }

  refetch = async () => {
    this.ord_loading(true)
    const { listData } = this.props
    const param = this.getQuery()
    try {
      const list = await listData(param)
      this.setState({ list })
    } catch (error) {
      logger.error(error)
    }
    this.ord_loading(false)
  }

  toDetailPage = id => {
    this.props.history.push(`/elips/${id}`)
  }

  addElip = () => {
    const { isLogin, history } = this.props
    if (!isLogin) {
      sessionStorage.setItem('loginRedirect', '/elips/new')
      history.push('/login')
    } else {
      history.push('/elips/new')
    }
  }

  searchChangedHandler = search => {
    this.props.updateFilters({ search })
    this.setState({ search: search.trim() }, this.debouncedRefetch)
  }

  handleFilter = () => {
    const { isVisitableFilter } = this.state
    this.setState({ isVisitableFilter: !isVisitableFilter })
  }

  handleSearchChange = e => {
    this.setState({ search: e.target.value })
  }

  handleStatusChange = filter => {
    this.setState({ filter })
  }

  handleCreationDateChange = creationDate => {
    this.setState({ creationDate })
  }

  handleAuthorChange = e => {
    this.setState({ author: e.target.value })
  }

  handleTypeChange = type => {
    this.setState({ type })
  }

  handleClearFilter = () => {
    const defaultFiltes = this.props.getDefaultFilters()
    this.setState({ ...defaultFiltes })
    this.props.clearFilters()
  }

  handleApplyFilter = () => {
    const {
      search,
      filter,
      creationDate,
      author,
      type
    } = this.state
    this.props.updateFilters({
      search,
      filter,
      creationDate,
      author,
      type
    })
    this.refetch()
  }

  setFilter = filter => {
    this.setState({ filter }, this.refetch)
  }

  renderAuthor = createdBy => {
    return userUtil.formatUsername(createdBy)
  }

  renderStatus = status => {
    return I18N.get(`elip.status.${status}`) || ''
  }

  renderCreatedAt = createdAt => {
    const lang = localStorage.getItem('lang') || 'en'
    return lang === 'en'
      ? moment(createdAt).format('MMM D, YYYY')
      : moment(createdAt).format('YYYY-MM-DD')
  }

  onSortByChanged = async sortBy => {
    await this.props.onSortByChanged(sortBy)
    await this.refetch()
  }

  renderSortActions() {
    const sortBy = this.props.sortBy || DEFAULT_SORT
    const SORT_BY_TEXT = {
      createdAt: I18N.get('elip.fields.createdAt'),
      updatedAt: I18N.get('elip.fields.updatedAt'),
    }
    return (
      <div style={{paddingBottom: 24}}>
        {I18N.get('elip.sort')}
        :
        {' '}
        <Select
          name="type"
          style={{ width: 200, marginLeft: 16 }}
          onChange={this.onSortByChanged}
          value={sortBy}
        >
          {_.map(SORT_BY, value => (
            <Select.Option key={value} value={value}>
              {SORT_BY_TEXT[value]}
            </Select.Option>
          ))}
        </Select>
      </div>
    )
  }

  ord_render() {
    const { isSecretary, isLogin } = this.props
    const { filter, creationDate, author, type, isVisitableFilter } = this.state
    const columns = [
      {
        title: I18N.get('elip.fields.number'),
        dataIndex: 'vid',
        render: (vid, item) => (
          <a
            className="tableLink"
            onClick={this.toDetailPage.bind(this, item._id)}
          >
            {vid && `#${vid}`}
          </a>
        )
      },
      {
        title: I18N.get('elip.fields.title'),
        dataIndex: 'title',
        width: '35%',
        render: (title, item) => (
          <a
            onClick={this.toDetailPage.bind(this, item._id)}
            className="tableLink"
          >
            {title}
          </a>
        )
      },
      {
        title: I18N.get('elip.fields.author'),
        dataIndex: 'createdBy',
        render: createdBy => this.renderAuthor(createdBy)
      },
      {
        title: I18N.get('elip.fields.status'),
        dataIndex: 'status',
        render: status => this.renderStatus(status)
      },
      {
        title: I18N.get('elip.fields.createdAt'),
        dataIndex: 'createdAt',
        render: createdAt => this.renderCreatedAt(createdAt)
      },
      {
        title: I18N.get('elip.fields.updatedAt'),
        dataIndex: 'updatedAt',
        render: updatedAt => this.renderCreatedAt(updatedAt)
      }
    ]

    const createBtn = (
      <Col lg={8} md={8} sm={12} xs={24} style={{ textAlign: 'right' }}>
        <StyledButton onClick={this.addElip} className="cr-btn cr-btn-primary">
          {I18N.get('elip.button.add')}
        </StyledButton>
      </Col>
    )

    const title = (
      <Col lg={8} md={8} sm={12} xs={24}>
        <h2
          style={{ textAlign: 'left', paddingBottom: 0 }}
          className="komu-a cr-title-with-icon"
        >
          {I18N.get('elip.header')}
        </h2>
      </Col>
    )

    const searchInput = (
      <Col lg={8} md={12} sm={12} xs={24}>
        <StyledSearch
          value={this.state.search}
          onChange={this.handleSearchChange}
          onSearch={this.searchChangedHandler}
          placeholder={I18N.get('developer.search.search.placeholder')}
        />
      </Col>
    )

    const lang = localStorage.getItem('lang') || 'en'
    const rangePickerOptions = {}
    if (lang === 'zh') {
      rangePickerOptions.locale = rangePickerLocale
    }
    const filterBtns = (
      <FilterLabel>
        <Row
          type="flex"
          gutter={10}
          align="middle"
          justify="start"
          onClick={this.handleFilter}
        >
          <Col>{I18N.get('elip.fields.filter')}</Col>
          <Col>{isVisitableFilter ? <UpIcon /> : <DownIcon />}</Col>
        </Row>
      </FilterLabel>
    )

    const filterPanel = (
      <FilterPanel>
        <Row type="flex" gutter={10} className="filter">
          <Col span={12} className="filter-panel">
            <FilterContent>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('elip.fields.elipStatus')}
                </FilterItemLabel>
                <Select
                  className="filter-input"
                  value={filter}
                  onChange={this.handleStatusChange}
                >
                  {isLogin
                    ? _.map(ELIP_STATUS, value => (
                      <Select.Option key={value} value={value}>
                        {I18N.get(`elip.status.${value}`)}
                      </Select.Option>
                    ))
                    : _.map(
                      [
                        ELIP_STATUS.DRAFT,
                        ELIP_STATUS.CANCELLED,
                        ELIP_STATUS.SUBMITTED_AS_PROPOSAL
                      ],
                      value => (
                        <Select.Option key={value} value={value}>
                          {I18N.get(`elip.status.${value}`)}
                        </Select.Option>
                      )
                    )}
                </Select>
              </FilterItem>
            </FilterContent>
          </Col>
          <Col span={12} className="filter-panel">
            <FilterContent>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('elip.fields.creationDate')}
                </FilterItemLabel>
                <RangePicker
                  className="filter-input"
                  onChange={this.handleCreationDateChange}
                  value={creationDate}
                  {...rangePickerOptions}
                />
              </FilterItem>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('elip.fields.author')}
                </FilterItemLabel>
                <Input
                  className="filter-input"
                  value={author}
                  onChange={this.handleAuthorChange}
                />
              </FilterItem>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('elip.fields.type')}
                </FilterItemLabel>
                <Select
                  className="filter-input"
                  value={type}
                  onChange={this.handleTypeChange}
                >
                  {_.map(ELIP_TYPE, value => (
                    <Select.Option key={value} value={value}>
                      {I18N.get(`elip.form.typeTitle.${value}`)}
                    </Select.Option>
                  ))}
                </Select>
              </FilterItem>
            </FilterContent>
          </Col>
        </Row>
        <Row type="flex" gutter={30} justify="center" className="filter-btn">
          <Col>
            <FilterClearBtn onClick={this.handleClearFilter}>
              {I18N.get('elip.button.clearFilter')}
            </FilterClearBtn>
          </Col>
          <Col>
            <Button
              className="cr-btn cr-btn-primary"
              onClick={this.handleApplyFilter}
            >
              {I18N.get('elip.button.applyFilter')}
            </Button>
          </Col>
        </Row>
      </FilterPanel>
    )
    const sortActionsNode = this.renderSortActions()

    const { list, loading } = this.state
    let dataCSV = []
    if (isSecretary) {
      const itemsCSV = _.map(list, v => [
        v.vid,
        v.title,
        this.renderAuthor(v.createdBy),
        this.renderStatus(v.status),
        _.replace(this.renderCreatedAt(v.createdAt), ',', ' ')
      ])
      dataCSV = _.concat(
        [
          [
            I18N.get('elip.fields.number'),
            I18N.get('elip.fields.title'),
            I18N.get('elip.fields.author'),
            I18N.get('elip.fields.status'),
            I18N.get('elip.fields.createdAt'),
            I18N.get('elip.fields.updatedAt')
          ]
        ],
        itemsCSV
      )
    }
    return (
      <Container>
        <Row type="flex" align="middle" justify="space-between">
          {title}
          {createBtn}
        </Row>
        <Row
          type="flex"
          align="middle"
          justify="start"
          gutter={40}
          style={{ marginTop: 20, marginBottom: 20 }}
        >
          {searchInput}
          {filterBtns}
        </Row>
        {isVisitableFilter && filterPanel}
        <Row type="flex" align="middle" justify="end">
          {isSecretary && (
            <CSVLink data={dataCSV} style={{ marginBottom: 16 }}>
              {I18N.get('elip.button.exportAsCSV')}
            </CSVLink>
          )}
        </Row>
        <Row
          type="flex"
          justify="space-between"
          align="middle"
          style={{ borderBottom: '1px solid #E5E5E5' }}
        >
          {sortActionsNode}
        </Row>
        <Table
          columns={columns}
          loading={loading}
          dataSource={list}
          rowKey={(record, index) => index}
        />
      </Container>
    )
  }
}
