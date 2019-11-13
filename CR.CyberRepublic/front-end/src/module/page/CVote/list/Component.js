import React from 'react'
import _ from 'lodash'
import moment from 'moment/moment'
import BaseComponent from '@/model/BaseComponent'
import {
  Table,
  Row,
  Col,
  Button,
  Select,
  Input,
  DatePicker,
  Checkbox
} from 'antd'
import rangePickerLocale from 'antd/es/date-picker/locale/zh_CN'
import { CSVLink } from 'react-csv'
import I18N from '@/I18N'
import { logger } from '@/util'
import { CVOTE_RESULT, CVOTE_STATUS } from '@/constant'
import VoteStats from '../stats/Component'
import userUtil from '@/util/user'
import { ReactComponent as UpIcon } from '@/assets/images/icon-up.svg'
import { ReactComponent as DownIcon } from '@/assets/images/icon-down.svg'

// style
import {
  Container,
  List,
  Item,
  ItemUndecided,
  StyledButton,
  StyledSearch,
  VoteFilter,
  FilterLabel,
  FilterPanel,
  FilterContent,
  FilterItem,
  FilterItemLabel,
  FilterClearBtn,
  CheckboxText
} from './style'

const { RangePicker } = DatePicker

const FILTERS = {
  ALL: 'all',
  UNVOTED: CVOTE_RESULT.UNDECIDED
}

export default class extends BaseComponent {
  constructor(p) {
    super(p)

    this.state = {
      list: [],
      isVisitableFilter: false,
      loading: true,
      voteResult: sessionStorage.getItem('voteResult') || FILTERS.ALL,
      search: sessionStorage.getItem('proposalSearch') || '',
      status: '',
      budgetRequested: '',
      hasTrackingMsg: false,
      isUnvotedByYou: false,
      creationDate: [],
      author: '',
      type: '',
      endsDate: [],
      page: 1
    }

    this.debouncedRefetch = _.debounce(this.refetch.bind(this), 300)
  }

  async componentDidMount() {
    this.refetch()
  }

  handleFilter = () => {
    const { isVisitableFilter } = this.state
    this.setState({ isVisitableFilter: !isVisitableFilter })
  }

  handleStatusChange = status => {
    this.setState({ status })
  }

  handleBudgetRequestedChange = budgetRequested => {
    this.setState({ budgetRequested })
  }

  handleHasTrackingMsgChange = e => {
    this.setState({ hasTrackingMsg: e.target.checked })
  }

  handleIsUnvotedByYouChange = e => {
    this.setState({ isUnvotedByYou: e.target.checked })
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

  handleEndsDateChange = endsDate => {
    this.setState({ endsDate })
  }

  handleClearFilter = () => {
    this.setState({
      status: '',
      budgetRequested: '',
      hasTrackingMsg: false,
      isUnvotedByYou: false,
      creationDate: [],
      author: '',
      type: '',
      endsDate: []
    })
  }

  handleApplyFilter = () => {
    this.refetch()
  }

  ord_render() {
    const PROPOSAL_TYPE = {
      1: I18N.get('council.voting.type.newMotion'),
      2: I18N.get('council.voting.type.motionAgainst'),
      3: I18N.get('council.voting.type.anythingElse'),
      4: I18N.get('council.voting.type.standardTrack'),
      5: I18N.get('council.voting.type.process'),
      6: I18N.get('council.voting.type.information')
    }
    const { canManage, isCouncil, isSecretary } = this.props
    const { isVisitableFilter } = this.state

    const columns = [
      {
        title: I18N.get('council.voting.number'),
        dataIndex: 'vid',
        render: (vid, item, index) => (
          <a
            className="tableLink"
            onClick={this.toDetailPage.bind(this, item._id)}
          >
            {`#${vid}`}
          </a>
        )
      },
      {
        title: I18N.get('council.voting.title'),
        dataIndex: 'title',
        width: '30%',
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
        title: I18N.get('council.voting.type'),
        dataIndex: 'type',
        render: (type, item) => PROPOSAL_TYPE[type]
      },
      {
        title: I18N.get('council.voting.author'),
        dataIndex: 'proposedBy'
      },
      {
        title: I18N.get('council.voting.votingEndsIn'),
        dataIndex: 'proposedAt',
        key: 'endsIn',
        render: (proposedAt, item) => this.renderEndsIn(item)
      },
      {
        title: I18N.get('council.voting.voteByCouncil'),
        render: (id, item) => this.voteDataByUser(item)
      },
      {
        title: I18N.get('council.voting.status'),
        render: (id, item) => this.renderStatus(item.status)
      },
      {
        title: I18N.get('council.voting.proposedAt'),
        dataIndex: 'proposedAt',
        render: (proposedAt, doc) =>
          this.renderProposed(doc.published, proposedAt || doc.createdAt)
      }
    ]

    if (canManage) {
      columns.splice(1, 0, {
        dataIndex: 'published',
        render: (published, item, index) =>
          published ? (
            <i className="fas fa-eye" />
          ) : (
            <i className="far fa-eye-slash" />
          )
      })
    }

    const statusIndicator = (
      <List>
        <Item status={CVOTE_RESULT.SUPPORT} />
        <span>{I18N.get('council.voting.type.support')}</span>
        <Item status={CVOTE_RESULT.REJECT} />
        <span>{I18N.get('council.voting.type.reject')}</span>
        <Item status={CVOTE_RESULT.ABSTENTION} />
        <span>{I18N.get('council.voting.type.abstention')}</span>
        <ItemUndecided status={CVOTE_RESULT.UNDECIDED} />
        <span>{I18N.get('council.voting.type.undecided')}</span>
      </List>
    )

    const createBtn = canManage && (
      <Row type="flex" align="middle" justify="end">
        <Col lg={8} md={12} sm={24} xs={24} style={{ textAlign: 'right' }}>
          <StyledButton
            onClick={this.createAndRedirect}
            className="cr-btn cr-btn-primary"
          >
            {I18N.get('from.CVoteForm.button.add')}
          </StyledButton>
        </Col>
      </Row>
    )

    const filterBtnGroup = (
      <Button.Group className="filter-group">
        <StyledButton
          className={
            (this.state.voteResult === FILTERS.ALL && 'selected') || ''
          }
          onClick={this.clearFilters}
        >
          {I18N.get('council.voting.voteResult.all')}
        </StyledButton>
        <StyledButton
          className={
            (this.state.voteResult === FILTERS.UNVOTED && 'selected') || ''
          }
          onClick={() => this.setFilter(FILTERS.UNVOTED)}
        >
          {I18N.get('council.voting.voteResult.unvoted')}
        </StyledButton>
      </Button.Group>
    )
    const title = (
      <Col lg={8} md={8} sm={12} xs={24}>
        <h2
          style={{ textAlign: 'left', paddingBottom: 0 }}
          className="komu-a cr-title-with-icon"
        >
          {I18N.get('council.voting.proposalList')}
        </h2>
      </Col>
    )
    const searchInput = (
      <Col lg={8} md={8} sm={12} xs={24}>
        <StyledSearch
          defaultValue={this.state.search}
          onSearch={this.searchChangedHandler}
          placeholder={I18N.get('developer.search.search.placeholder')}
        />
      </Col>
    )
    const btns = (
      <Col lg={8} md={8} sm={12} xs={24}>
        {statusIndicator}
        {isCouncil && (
          <VoteFilter>
            <span>{`${I18N.get('council.voting.voteResult.show')}: `}</span>
            {filterBtnGroup}
          </VoteFilter>
        )}
      </Col>
    )
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
    const filterPanel = this.renderFilterPanel(PROPOSAL_TYPE)
    const { list, loading, page } = this.state
    let dataCSV = []
    if (isSecretary) {
      const itemsCSV = _.map(list, v => [
        v.vid,
        v.title,
        PROPOSAL_TYPE[v.type],
        v.proposedBy,
        this.renderEndsInForCSV(v),
        this.voteDataByUserForCSV(v),
        this.renderStatus(v.status),
        _.replace(
          this.renderProposed(v.published, v.proposedAt || v.createdAt) || '',
          ',',
          ' '
        )
      ])
      dataCSV = _.concat(
        [
          [
            I18N.get('council.voting.number'),
            I18N.get('council.voting.title'),
            I18N.get('council.voting.type'),
            I18N.get('council.voting.author'),
            I18N.get('council.voting.votingEndsIn'),
            I18N.get('council.voting.voteByCouncil'),
            I18N.get('council.voting.status'),
            I18N.get('council.voting.proposedAt')
          ]
        ],
        itemsCSV
      )
    }
    return (
      <Container>
        {createBtn}
        <Row
          type="flex"
          align="middle"
          justify="space-between"
          style={{ marginTop: 20 }}
        >
          {title}
          {btns}
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
        <Table
          columns={columns}
          loading={loading}
          dataSource={list}
          rowKey={record => record._id}
          pagination={{
            current: page,
            pageSize: 10,
            total: list && list.length,
            onChange: this.onPageChange
          }}
        />
        {createBtn}
      </Container>
    )
  }

  onPageChange = (page, pageSize) => {
    this.setState({ page: parseInt(page) })
    sessionStorage.setItem('proposalPage', page)
  }

  createAndRedirect = async () => {
    sessionStorage.removeItem('proposalPage')
    sessionStorage.removeItem('voteResult')
    sessionStorage.removeItem('proposalSearch')

    const { user } = this.props
    const fullName = userUtil.formatUsername(user)
    const { createDraft } = this.props

    const param = {
      title: 'New Proposal',
      proposedBy: fullName,
      proposer: user._id
    }

    this.ord_loading(true)

    try {
      const res = await createDraft(param)
      this.ord_loading(false)
      this.toEditPage(res._id)
    } catch (error) {
      this.ord_loading(false)
    }
  }

  getQuery = () => {
    const {
      status,
      budgetRequested,
      hasTrackingMsg,
      isUnvotedByYou,
      creationDate,
      author,
      type,
      endsDate
    } = this.state
    const query = {}
    const voteResult =
      sessionStorage.getItem('voteResult') || this.state.voteResult
    query.voteResult = voteResult
    const searchStr =
      this.state.search || sessionStorage.getItem('proposalSearch')
    const formatStr = 'YYYY-MM-DD'
    if (searchStr) {
      query.search = searchStr
    }
    if (!_.isEmpty(status)) {
      query.status = status
    }
    if (!_.isEmpty(budgetRequested)) {
      query.budget = budgetRequested
    }
    if (hasTrackingMsg) {
      query.hasTracking = hasTrackingMsg
    }
    if (isUnvotedByYou) {
      query.unvoted = isUnvotedByYou
    }
    if (!_.isEmpty(creationDate)) {
      query.startDate = moment(creationDate[0]).format(formatStr)
      query.endDate = moment(creationDate[1]).format(formatStr)
    }
    if (!_.isEmpty(author)) {
      query.author = author
    }
    if (!_.isEmpty(type)) {
      query.type = type
    }
    if (!_.isEmpty(endsDate)) {
      query.endsInStartDate = moment(endsDate[0]).format(formatStr)
      query.endsInEndDate = moment(endsDate[1]).format(formatStr)
    }
    return query
  }

  refetch = async () => {
    this.ord_loading(true)
    const { listData, canManage } = this.props
    const param = this.getQuery()
    try {
      const list = await listData(param, canManage)
      const page = sessionStorage.getItem('proposalPage')
      this.setState({ list, page: (page && parseInt(page)) || 1 })
    } catch (error) {
      logger.error(error)
    }

    this.ord_loading(false)
  }

  searchChangedHandler = search => {
    sessionStorage.removeItem('proposalPage')
    sessionStorage.setItem('proposalSearch', search)
    this.setState({ search }, this.debouncedRefetch)
  }

  onFilterChanged = value => {
    switch (value) {
      case FILTERS.ALL:
        this.clearFilters()
        break
      case FILTERS.UNVOTED:
        this.setFilter(FILTERS.UNVOTED)
        break
      default:
        this.clearFilters()
        break
    }
  }

  clearFilters = () => {
    sessionStorage.removeItem('proposalPage')
    sessionStorage.setItem('voteResult', FILTERS.ALL)
    this.setState({ voteResult: FILTERS.ALL }, this.refetch)
  }

  setFilter = voteResult => {
    sessionStorage.removeItem('proposalPage')
    sessionStorage.setItem('voteResult', voteResult)
    this.setState({ voteResult }, this.refetch)
  }

  toDetailPage(id) {
    this.props.history.push(`/proposals/${id}`)
  }

  toEditPage(id) {
    this.props.history.push(`/proposals/${id}/edit`)
  }

  renderEndsIn = item => {
    return this.renderBaseEndsIn(item)
  }

  renderEndsInForCSV = item => {
    return this.renderBaseEndsIn(item, true)
  }

  renderBaseEndsIn = (item, isCSV = false) => {
    if (item.status === CVOTE_STATUS.DRAFT) return null
    // only show when status is PROPOSED
    const endsInFloat = moment
      .duration(
        moment(item.proposedAt || item.createdAt)
          .add(7, 'd')
          .diff(moment())
      )
      .as('days')
    if (item.status !== CVOTE_STATUS.PROPOSED || endsInFloat <= 0) {
      return I18N.get('council.voting.votingEndsIn.ended')
    }
    if (endsInFloat > 0 && endsInFloat <= 1) {
      const oneDay = `1 ${I18N.get('council.voting.votingEndsIn.day')}`
      return isCSV ? oneDay : <span style={{ color: 'red' }}>{oneDay}</span>
    }
    return `${Math.ceil(endsInFloat)} ${I18N.get(
      'council.voting.votingEndsIn.days'
    )}`
  }

  renderStatus = status => {
    return I18N.get(`cvoteStatus.${status}`) || ''
  }

  renderProposed = (published, createdAt) => {
    const lang = localStorage.getItem('lang') || 'en'
    const format = lang === 'en' ? 'MMM D, YYYY' : 'YYYY-MM-DD'
    return published && moment(createdAt).format(format)
  }

  voteDataByUser = data => {
    return this.baseVoteDataByUser(data)
  }

  voteDataByUserForCSV = data => {
    return this.baseVoteDataByUser(data, true)
  }

  baseVoteDataByUser = (data, isCSV = false) => {
    const { vote_map: voteMap, voteResult, status } = data
    let voteArr

    if (status === CVOTE_STATUS.DRAFT) return null

    if (!_.isEmpty(voteResult)) {
      voteArr = _.map(
        voteResult,
        item => CVOTE_RESULT[item.value.toUpperCase()]
      )
    } else if (!_.isEmpty(voteMap)) {
      voteArr = _.map(
        voteMap,
        value => CVOTE_RESULT[value.toUpperCase()] || CVOTE_RESULT.UNDECIDED
      )
    } else {
      return ''
    }
    const supportNum = _.countBy(voteArr)[CVOTE_RESULT.SUPPORT] || 0
    const percentage = (supportNum * 100) / voteArr.length
    const proposalAgreed = percentage > 50
    const percentageStr =
      percentage.toString() && `${percentage.toFixed(1).toString()}%`
    return isCSV ? (
      percentageStr
    ) : (
      <VoteStats
        percentage={percentageStr}
        values={voteArr}
        yes={proposalAgreed}
      />
    )
  }

  renderFilterPanel = (PROPOSAL_TYPE) => {
    const {
      status,
      budgetRequested,
      hasTrackingMsg,
      isUnvotedByYou,
      creationDate,
      author,
      type,
      endsDate
    } = this.state
    const lang = localStorage.getItem('lang') || 'en'
    const rangePickerOptions = {}
    if (lang === 'zh') {
      rangePickerOptions.locale = rangePickerLocale
    }
    const budgetRequestedOptions = {
      1: '$0 - $100 (USD)',
      2: '$100 - $1000 (USD)',
      3: '> $1000 (USD)'
    }
    return (
      <FilterPanel>
        <Row type="flex" gutter={10} className="filter">
          <Col span={8} className="filter-panel">
            <FilterContent>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('proposal.fields.status')}
                </FilterItemLabel>
                <Select
                  className="filter-input"
                  value={status}
                  onChange={this.handleStatusChange}
                >
                  {_.map(CVOTE_STATUS, value => (
                    <Select.Option key={value} value={value}>
                      {I18N.get(`cvoteStatus.${value}`)}
                    </Select.Option>
                  ))}
                </Select>
              </FilterItem>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('proposal.fields.budgetRequested')}
                </FilterItemLabel>
                <Select
                  className="filter-input"
                  value={budgetRequested}
                  onChange={this.handleBudgetRequestedChange}
                >
                  {_.map(budgetRequestedOptions, value => (
                    <Select.Option key={value} value={value}>
                      {value}
                    </Select.Option>
                  ))}
                </Select>
              </FilterItem>
              <FilterItem className="filter-checkbox">
                <Checkbox
                  checked={hasTrackingMsg}
                  onChange={this.handleHasTrackingMsgChange}
                />
                <CheckboxText>
                  {I18N.get('proposal.fields.hasTrackingMsg')}
                </CheckboxText>
              </FilterItem>
            </FilterContent>
          </Col>
          <Col span={8} className="filter-panel">
            <FilterContent>
              <FilterItem>
                <Checkbox
                  checked={isUnvotedByYou}
                  onChange={this.handleIsUnvotedByYouChange}
                />
                <CheckboxText>
                  {I18N.get('proposal.fields.isUnvotedByYou')}
                </CheckboxText>
              </FilterItem>
            </FilterContent>
          </Col>
          <Col span={8} className="filter-panel">
            <FilterContent>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('proposal.fields.creationDate')}
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
                  {I18N.get('proposal.fields.author')}
                </FilterItemLabel>
                <Input
                  className="filter-input"
                  value={author}
                  onChange={this.handleAuthorChange}
                />
              </FilterItem>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('proposal.fields.type')}
                </FilterItemLabel>
                <Select
                  className="filter-input"
                  value={type}
                  onChange={this.handleTypeChange}
                >
                  {_.map(PROPOSAL_TYPE, (value, key) => (
                    <Select.Option key={key} value={key}>
                      {value}
                    </Select.Option>
                  ))}
                </Select>
              </FilterItem>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('proposal.fields.endsDate')}
                </FilterItemLabel>
                <RangePicker
                  className="filter-input"
                  onChange={this.handleEndsDateChange}
                  value={endsDate}
                  {...rangePickerOptions}
                />
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
  }
}
